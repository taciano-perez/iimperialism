    .importzp c_sp, ptr1, ptr2, ptr3      ; cc65 software stack and temporary ZP pointers

    .import _font_data                    ; exported 96x8 glyph table from font.h/ui.c
    .import _font_bold_data               ; exported 96x8 bold glyph table from font.h/ui.c

    .export _draw_text_hgr_opaque         ; callable from C via print()
    .export _draw_text_hgr_opaque_bold    ; callable from C via print_bold()
    .export _draw_text_hgr_opaque_inverted ; callable from C via print_inverted()

    .segment "BSS"

text_start:
    .res 2                               ; original text pointer so each row can rescan the string

base_addr:
    .res 2                               ; leftmost HGR address for the first row of this text line

x_byte_tmp:
    .res 1                               ; character-cell X position, already aligned to HGR bytes

row_index:
    .res 1                               ; glyph row 0..7 currently being emitted

    .segment "RODATA"

CHAR_ROW_BASE_LO:
    ; Low bytes for HGR row addresses at pixel rows 0,8,16,...,184.
    .byte $00, $80, $00, $80, $00, $80, $00, $80, $28, $A8, $28, $A8, $28, $A8, $28, $A8, $50, $D0, $50, $D0, $50, $D0, $50, $D0

CHAR_ROW_BASE_HI:
    ; High bytes for HGR row addresses at pixel rows 0,8,16,...,184.
    .byte $20, $20, $21, $21, $22, $22, $23, $23, $20, $20, $21, $21, $22, $22, $23, $23, $20, $20, $21, $21, $22, $22, $23, $23

REV7:
    ; Reverse bits 0..6 so font rows encoded for tgi_setpixel order become HGR byte order.
    .byte $00, $40, $20, $60, $10, $50, $30, $70, $08, $48, $28, $68, $18, $58, $38, $78
    .byte $04, $44, $24, $64, $14, $54, $34, $74, $0C, $4C, $2C, $6C, $1C, $5C, $3C, $7C
    .byte $02, $42, $22, $62, $12, $52, $32, $72, $0A, $4A, $2A, $6A, $1A, $5A, $3A, $7A
    .byte $06, $46, $26, $66, $16, $56, $36, $76, $0E, $4E, $2E, $6E, $1E, $5E, $3E, $7E
    .byte $01, $41, $21, $61, $11, $51, $31, $71, $09, $49, $29, $69, $19, $59, $39, $79
    .byte $05, $45, $25, $65, $15, $55, $35, $75, $0D, $4D, $2D, $6D, $1D, $5D, $3D, $7D
    .byte $03, $43, $23, $63, $13, $53, $33, $73, $0B, $4B, $2B, $6B, $1B, $5B, $3B, $7B
    .byte $07, $47, $27, $67, $17, $57, $37, $77, $0F, $4F, $2F, $6F, $1F, $5F, $3F, $7F

    .segment "LOWCODE"

; void draw_text_hgr_opaque(const char* text, unsigned char x_byte, unsigned char y_char);
; cc65 default calling convention:
;   A   = y_char
;   sp+0 = x_byte
;   sp+1 = text_lo
;   sp+2 = text_hi
_draw_text_hgr_opaque:
    tax                                 ; keep y_char in X for table indexing

    ldy     #$00                        ; start reading stacked arguments at sp+0
    lda     (c_sp),y                    ; load x_byte
    sta     x_byte_tmp                  ; save aligned X position
    iny                                 ; advance to text_lo
    lda     (c_sp),y                    ; load text pointer low byte
    sta     text_start                  ; save original text pointer low byte
    iny                                 ; advance to text_hi
    lda     (c_sp),y                    ; load text pointer high byte
    sta     text_start+1                ; save original text pointer high byte

    lda     c_sp                        ; pop three stacked bytes: x + text pointer
    clc                                 ; prepare 16-bit add
    adc     #$03                        ; advance stack low byte by 3
    sta     c_sp                        ; store updated stack low byte
    bcc     :+                          ; skip carry fixup if low byte did not wrap
    inc     c_sp+1                      ; carry into stack high byte when needed
:
    lda     CHAR_ROW_BASE_LO,x          ; load low byte for this character row's first HGR line
    clc                                 ; prepare addition with x_byte
    adc     x_byte_tmp                  ; add horizontal byte offset
    sta     base_addr                   ; store resulting destination low byte
    lda     CHAR_ROW_BASE_HI,x          ; load high byte for this character row's first HGR line
    adc     #$00                        ; fold in carry from low-byte add
    sta     base_addr+1                 ; store resulting destination high byte

    lda     #$00                        ; start at glyph row 0
    sta     row_index                   ; initialize row loop counter

@row_loop:
    lda     text_start                  ; restore original text pointer low byte
    sta     ptr2                        ; ptr2 will scan the string for this row
    lda     text_start+1                ; restore original text pointer high byte
    sta     ptr2+1                      ; complete ptr2 reset for this row

    lda     base_addr                   ; row address low byte is unchanged within a text line
    sta     ptr1                        ; ptr1 is the current screen write pointer
    lda     row_index                   ; load row number 0..7
    asl     a                           ; multiply by 2
    asl     a                           ; multiply by 4, matching Apple II's $0400 row stride
    clc                                 ; prepare add with base high byte
    adc     base_addr+1                 ; add row-specific HGR page offset
    sta     ptr1+1                      ; set current row destination high byte

@char_loop:
    ldy     #$00                        ; always dereference text/glyph pointers at offset 0 or row_index
    lda     (ptr2),y                    ; load next character from the string
    beq     @next_row                   ; end of string: move on to the next glyph row

    inc     ptr2                        ; advance text pointer low byte
    bne     :+                          ; skip high-byte fixup if no wrap
    inc     ptr2+1                      ; carry text pointer into next page
:
    cmp     #$20                        ; below ASCII 32 maps to space
    bcc     @store_blank                ; blank unsupported low characters
    cmp     #$80                        ; above ASCII 127 maps to space
    bcs     @store_blank                ; blank unsupported high characters
    sec                                 ; convert ASCII code to glyph index by subtracting 32
    sbc     #$20                        ; A now holds 0..95

    sta     ptr3                        ; start 16-bit glyph offset in ptr3 low byte
    lda     #$00                        ; glyph offset high byte starts at zero
    sta     ptr3+1                      ; clear high byte before shifting
    asl     ptr3                        ; multiply glyph index by 2
    rol     ptr3+1                      ; propagate carry into high byte
    asl     ptr3                        ; multiply glyph index by 4
    rol     ptr3+1                      ; propagate carry into high byte
    asl     ptr3                        ; multiply glyph index by 8 (bytes per glyph)
    rol     ptr3+1                      ; propagate carry into high byte
    lda     ptr3                        ; load glyph offset low byte
    clc                                 ; prepare add with font_data base address
    adc     #<(_font_data)              ; add font table low byte
    sta     ptr3                        ; ptr3 now points at glyph start low byte
    lda     ptr3+1                      ; load glyph offset high byte
    adc     #>(_font_data)              ; add font table high byte and carry
    sta     ptr3+1                      ; ptr3 now points at glyph start high byte

    ldy     row_index                   ; select the current row within the glyph
    lda     (ptr3),y                    ; load the 7-bit font row
    tax                                 ; use row value as lookup index
    lda     REV7,x                      ; convert bit order to HGR byte order
    bne     @store_byte                 ; nonzero bytes can be written directly

@store_blank:
    lda     #$00                        ; blank characters emit a zero byte

@store_byte:
    ldy     #$00                        ; indirect store always uses offset 0
    sta     (ptr1),y                    ; write one opaque HGR byte for this character row
    inc     ptr1                        ; advance destination low byte to the next character cell
    bne     @char_loop                  ; continue row if the pointer stayed in-page
    inc     ptr1+1                      ; otherwise carry into the next page
    jmp     @char_loop                  ; resume scanning characters for this row

@next_row:
    inc     row_index                   ; advance to the next glyph row
    lda     row_index                   ; load updated row count
    cmp     #$08                        ; stop after rows 0..7
    bcc     @row_loop                   ; more rows remain, so restart with the original text
    rts                                 ; all eight rows rendered

; void draw_text_hgr_opaque_bold(const char* text, unsigned char x_byte, unsigned char y_char);
; cc65 default calling convention:
;   A   = y_char
;   sp+0 = x_byte
;   sp+1 = text_lo
;   sp+2 = text_hi
_draw_text_hgr_opaque_bold:
    tax                                 ; keep y_char in X for table indexing

    ldy     #$00                        ; start reading stacked arguments at sp+0
    lda     (c_sp),y                    ; load x_byte
    sta     x_byte_tmp                  ; save aligned X position
    iny                                 ; advance to text_lo
    lda     (c_sp),y                    ; load text pointer low byte
    sta     text_start                  ; save original text pointer low byte
    iny                                 ; advance to text_hi
    lda     (c_sp),y                    ; load text pointer high byte
    sta     text_start+1                ; save original text pointer high byte

    lda     c_sp                        ; pop three stacked bytes: x + text pointer
    clc                                 ; prepare 16-bit add
    adc     #$03                        ; advance stack low byte by 3
    sta     c_sp                        ; store updated stack low byte
    bcc     :+                          ; skip carry fixup if low byte did not wrap
    inc     c_sp+1                      ; carry into stack high byte when needed
:
    lda     CHAR_ROW_BASE_LO,x          ; load low byte for this character row's first HGR line
    clc                                 ; prepare addition with x_byte
    adc     x_byte_tmp                  ; add horizontal byte offset
    sta     base_addr                   ; store resulting destination low byte
    lda     CHAR_ROW_BASE_HI,x          ; load high byte for this character row's first HGR line
    adc     #$00                        ; fold in carry from low-byte add
    sta     base_addr+1                 ; store resulting destination high byte

    lda     #$00                        ; start at glyph row 0
    sta     row_index                   ; initialize row loop counter

@row_loop_bold:
    lda     text_start                  ; restore original text pointer low byte
    sta     ptr2                        ; ptr2 will scan the string for this row
    lda     text_start+1                ; restore original text pointer high byte
    sta     ptr2+1                      ; complete ptr2 reset for this row

    lda     base_addr                   ; row address low byte is unchanged within a text line
    sta     ptr1                        ; ptr1 is the current screen write pointer
    lda     row_index                   ; load row number 0..7
    asl     a                           ; multiply by 2
    asl     a                           ; multiply by 4, matching Apple II's $0400 row stride
    clc                                 ; prepare add with base high byte
    adc     base_addr+1                 ; add row-specific HGR page offset
    sta     ptr1+1                      ; set current row destination high byte

@char_loop_bold:
    ldy     #$00                        ; always dereference text/glyph pointers at offset 0 or row_index
    lda     (ptr2),y                    ; load next character from the string
    beq     @next_row_bold              ; end of string: move on to the next glyph row

    inc     ptr2                        ; advance text pointer low byte
    bne     :+                          ; skip high-byte fixup if no wrap
    inc     ptr2+1                      ; carry text pointer into next page
:
    cmp     #$20                        ; below ASCII 32 maps to space
    bcc     @store_blank_bold           ; blank unsupported low characters
    cmp     #$80                        ; above ASCII 127 maps to space
    bcs     @store_blank_bold           ; blank unsupported high characters
    sec                                 ; convert ASCII code to glyph index by subtracting 32
    sbc     #$20                        ; A now holds 0..95

    sta     ptr3                        ; start 16-bit glyph offset in ptr3 low byte
    lda     #$00                        ; glyph offset high byte starts at zero
    sta     ptr3+1                      ; clear high byte before shifting
    asl     ptr3                        ; multiply glyph index by 2
    rol     ptr3+1                      ; propagate carry into high byte
    asl     ptr3                        ; multiply glyph index by 4
    rol     ptr3+1                      ; propagate carry into high byte
    asl     ptr3                        ; multiply glyph index by 8 (bytes per glyph)
    rol     ptr3+1                      ; propagate carry into high byte
    lda     ptr3                        ; load glyph offset low byte
    clc                                 ; prepare add with font_bold_data base address
    adc     #<(_font_bold_data)         ; add bold font table low byte
    sta     ptr3                        ; ptr3 now points at glyph start low byte
    lda     ptr3+1                      ; load glyph offset high byte
    adc     #>(_font_bold_data)         ; add bold font table high byte and carry
    sta     ptr3+1                      ; ptr3 now points at glyph start high byte

    ldy     row_index                   ; select the current row within the glyph
    lda     (ptr3),y                    ; load the 7-bit font row
    tax                                 ; use row value as lookup index
    lda     REV7,x                      ; convert bit order to HGR byte order
    bne     @store_byte_bold            ; nonzero bytes can be written directly

@store_blank_bold:
    lda     #$00                        ; blank characters emit a zero byte

@store_byte_bold:
    ldy     #$00                        ; indirect store always uses offset 0
    sta     (ptr1),y                    ; write one opaque HGR byte for this character row
    inc     ptr1                        ; advance destination low byte to the next character cell
    bne     @char_loop_bold             ; continue row if the pointer stayed in-page
    inc     ptr1+1                      ; otherwise carry into the next page
    jmp     @char_loop_bold             ; resume scanning characters for this row

@next_row_bold:
    inc     row_index                   ; advance to the next glyph row
    lda     row_index                   ; load updated row count
    cmp     #$08                        ; stop after rows 0..7
    bcc     @row_loop_bold              ; more rows remain, so restart with the original text
    rts                                 ; all eight rows rendered

; void draw_text_hgr_opaque_inverted(const char* text, unsigned char x_byte, unsigned char y_char);
; cc65 default calling convention:
;   A   = y_char
;   sp+0 = x_byte
;   sp+1 = text_lo
;   sp+2 = text_hi
_draw_text_hgr_opaque_inverted:
    tax                                 ; keep y_char in X for table indexing

    ldy     #$00                        ; start reading stacked arguments at sp+0
    lda     (c_sp),y                    ; load x_byte
    sta     x_byte_tmp                  ; save aligned X position
    iny                                 ; advance to text_lo
    lda     (c_sp),y                    ; load text pointer low byte
    sta     text_start                  ; save original text pointer low byte
    iny                                 ; advance to text_hi
    lda     (c_sp),y                    ; load text pointer high byte
    sta     text_start+1                ; save original text pointer high byte

    lda     c_sp                        ; pop three stacked bytes: x + text pointer
    clc                                 ; prepare 16-bit add
    adc     #$03                        ; advance stack low byte by 3
    sta     c_sp                        ; store updated stack low byte
    bcc     :+                          ; skip carry fixup if low byte did not wrap
    inc     c_sp+1                      ; carry into stack high byte when needed
:
    lda     CHAR_ROW_BASE_LO,x          ; load low byte for this character row's first HGR line
    clc                                 ; prepare addition with x_byte
    adc     x_byte_tmp                  ; add horizontal byte offset
    sta     base_addr                   ; store resulting destination low byte
    lda     CHAR_ROW_BASE_HI,x          ; load high byte for this character row's first HGR line
    adc     #$00                        ; fold in carry from low-byte add
    sta     base_addr+1                 ; store resulting destination high byte

    lda     #$00                        ; start at glyph row 0
    sta     row_index                   ; initialize row loop counter

@row_loop_inverted:
    lda     text_start                  ; restore original text pointer low byte
    sta     ptr2                        ; ptr2 will scan the string for this row
    lda     text_start+1                ; restore original text pointer high byte
    sta     ptr2+1                      ; complete ptr2 reset for this row

    lda     base_addr                   ; row address low byte is unchanged within a text line
    sta     ptr1                        ; ptr1 is the current screen write pointer
    lda     row_index                   ; load row number 0..7
    asl     a                           ; multiply by 2
    asl     a                           ; multiply by 4, matching Apple II's $0400 row stride
    clc                                 ; prepare add with base high byte
    adc     base_addr+1                 ; add row-specific HGR page offset
    sta     ptr1+1                      ; set current row destination high byte

@char_loop_inverted:
    ldy     #$00                        ; always dereference text/glyph pointers at offset 0 or row_index
    lda     (ptr2),y                    ; load next character from the string
    beq     @next_row_inverted          ; end of string: move on to the next glyph row

    inc     ptr2                        ; advance text pointer low byte
    bne     :+                          ; skip high-byte fixup if no wrap
    inc     ptr2+1                      ; carry text pointer into next page
:
    cmp     #$20                        ; below ASCII 32 maps to space
    bcc     @store_blank_inverted       ; blank unsupported low characters
    cmp     #$80                        ; above ASCII 127 maps to space
    bcs     @store_blank_inverted       ; blank unsupported high characters
    sec                                 ; convert ASCII code to glyph index by subtracting 32
    sbc     #$20                        ; A now holds 0..95

    sta     ptr3                        ; start 16-bit glyph offset in ptr3 low byte
    lda     #$00                        ; glyph offset high byte starts at zero
    sta     ptr3+1                      ; clear high byte before shifting
    asl     ptr3                        ; multiply glyph index by 2
    rol     ptr3+1                      ; propagate carry into high byte
    asl     ptr3                        ; multiply glyph index by 4
    rol     ptr3+1                      ; propagate carry into high byte
    asl     ptr3                        ; multiply glyph index by 8 (bytes per glyph)
    rol     ptr3+1                      ; propagate carry into high byte
    lda     ptr3                        ; load glyph offset low byte
    clc                                 ; prepare add with font_data base address
    adc     #<(_font_data)              ; add font table low byte
    sta     ptr3                        ; ptr3 now points at glyph start low byte
    lda     ptr3+1                      ; load glyph offset high byte
    adc     #>(_font_data)              ; add font table high byte and carry
    sta     ptr3+1                      ; ptr3 now points at glyph start high byte

    ldy     row_index                   ; select the current row within the glyph
    lda     (ptr3),y                    ; load the 7-bit font row
    tax                                 ; use row value as lookup index
    lda     REV7,x                      ; convert bit order to HGR byte order
    eor     #$7F                        ; invert only the 7 visible pixels, keep color bit clear
    jmp     @store_byte_inverted

@store_blank_inverted:
    lda     #$7F                        ; a blank glyph becomes a solid white cell

@store_byte_inverted:
    ldy     #$00                        ; indirect store always uses offset 0
    sta     (ptr1),y                    ; write one opaque HGR byte for this character row
    inc     ptr1                        ; advance destination low byte to the next character cell
    bne     @char_loop_inverted         ; continue row if the pointer stayed in-page
    inc     ptr1+1                      ; otherwise carry into the next page
    jmp     @char_loop_inverted         ; resume scanning characters for this row

@next_row_inverted:
    inc     row_index                   ; advance to the next glyph row
    lda     row_index                   ; load updated row count
    cmp     #$08                        ; stop after rows 0..7
    bcc     @row_loop_inverted          ; more rows remain, so restart with the original text
    rts                                 ; all eight rows rendered
