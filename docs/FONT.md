# Font Conversion Process

This document describes the Taipan-derived bitmap fonts stored in `include/font.h`
and the runtime path that renders text on the Apple II HGR screen.

## Overview

The current font pipeline supports two runtime fonts:

1. regular font from `assets/img/taipan-font.png`
2. bold font from `assets/img/taipan-font-bold.png`

For both fonts, the pipeline:

1. extracts glyphs from a source PNG
2. downscales them from `14x16` to `7x8`
3. packs each 7-pixel row into one byte
4. emits the glyph table used by the game

Runtime rendering is handled separately by the assembly blitter in
`asm/text_hgr.s`.

## Source Font Format

### Regular Font: `assets/img/taipan-font.png`

- Dimensions: `258x110`
- Grid: `6 x 16 = 96` characters
- Character mapping: ASCII 32 (`' '`) through ASCII 127
- Character order: left-to-right, top-to-bottom

```text
Row 1: <space> ! " # $ % & ' ( ) * + , - . /
Row 2: 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
Row 3: @ A B C D E F G H I J K L M N O
Row 4: P Q R S T U V W X Y Z [ \ ] ^ _
Row 5: ` a b c d e f g h i j k l m n o
Row 6: p q r s t u v w x y z { | } ~ <block>
```

### Cell Structure

Each source cell is:

- Glyph area: `14x16`
- Border between cells: `2` pixels
- Outer border: `2` pixels
- Effective cell spacing: `16x18`

Extraction coordinates for one glyph:

```text
x_start = 2 + col * (14 + 2)
y_start = 2 + row * (16 + 2)
```

### Bold Font: `assets/img/taipan-font-bold.png`

- Dimensions: `514x110`
- Grid present in the image: `6 x 32 = 192` characters
- Rows currently used by the game: first `3`
- Runtime glyph coverage taken from those rows: `3 x 32 = 96` characters
- Character mapping: ASCII 32 (`' '`) through ASCII 127
- Character order: left-to-right, top-to-bottom

The bold source sheet uses the same per-glyph geometry as the regular font:

- Glyph area: `14x16`
- Border between cells: `2` pixels
- Outer border: `2` pixels
- Effective cell spacing: `16x18`

Extraction coordinates for one bold glyph:

```text
x_start = 2 + col * (14 + 2)
y_start = 2 + row * (16 + 2)
```

Examples:

- first glyph: `(2, 2)`
- second glyph: `(18, 2)`

The checked-in bold source contains blank glyphs for `|`, `}`, and `~`. That is
intentional for the current asset and is reflected in the generated runtime
table.

## Target Font Format

### Glyph Size

- Dimensions: `7x8`
- Storage: `font_data[96][8]` and `font_bold_data[96][8]`
- One byte per glyph row
- Only bits `0..6` are used for glyph pixels

In `include/font.h` the runtime font assets are:

```c
#define FONT_FIRST_CHAR 32
#define FONT_LAST_CHAR 127
#define FONT_GLYPH_WIDTH 7
#define FONT_GLYPH_HEIGHT 8

const unsigned char font_data[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x08, 0x00},
    /* ... */
};

const unsigned char font_bold_data[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00},
    /* ... */
};
```

## Downscaling Method

The source font is exactly double the target size in both dimensions, so the
conversion uses a simple `2:1` reduction.

For each target pixel `(tx, ty)`:

1. Inspect the source `2x2` block at `(tx * 2, ty * 2)`
2. If any source pixel in that block is on, set the target pixel on
3. Otherwise leave it off

This preserves thin strokes better than averaging for this specific source.

## Byte Packing

Each 7-pixel row becomes one byte in the generated font tables.

- Bit 6 represents the leftmost source pixel
- Bit 0 represents the rightmost source pixel

Example:

```text
#..##.#
```

becomes:

```text
1001101b = 0x4D
```

## Extraction Scripts

The font extraction scripts produce the glyph tables that get copied into
`include/font.h`.

Current in-tree script:

- `tools/extract_font_bold.py`

Typical usage:

```bash
python tools/extract_font_bold.py > font_bold_output.txt
```

The bold extraction script:

- reads `assets/img/taipan-font-bold.png`
- extracts the first `96` glyphs from the first `3` rows
- applies the `2:1` reduction to `7x8`
- emits `font_bold_data[96][8]`

It does not generate the runtime renderer.

There is currently no checked-in extraction script for the regular font, even
though the regular font data in `include/font.h` follows the same packed
runtime format.

## Runtime Rendering

### Current Paths

The current text paths are:

1. `print()` in `src/ui.c`
2. `draw_text_hgr_opaque()` in `asm/text_hgr.s`
3. direct HGR byte writes using `font_data`

and

1. `print_bold()` in `src/ui.c`
2. `draw_text_hgr_opaque_bold()` in `asm/text_hgr.s`
3. direct HGR byte writes using `font_bold_data`

Relevant entry point:

```c
void print(unsigned char x, unsigned char y, const char* text) {
    draw_text_hgr_opaque(text, x, y);
}

void print_bold(unsigned char x, unsigned char y, const char* text) {
    draw_text_hgr_opaque_bold(text, x, y);
}
```

### Coordinate System

The text renderer uses character-grid coordinates, not pixel coordinates:

- `x`: `0..39`, one 7-pixel HGR byte per column
- `y`: `0..23`, one 8-pixel glyph row per text row

This matches the UI constants in `src/ui.c`:

- `CHAR_WIDTH = 7`
- `CHAR_HEIGHT = 8`

### What `asm/text_hgr.s` Does

The assembly blitter:

- assumes byte-aligned HGR text placement
- scans the string once per glyph row
- maps ASCII 32..127 into either `font_data[96][8]` or
  `font_bold_data[96][8]`
- reverses glyph bits with the `REV7` lookup table before writing
- writes opaque bytes, so existing pixels under text are overwritten

The bit reversal step is required because the stored glyph rows and Apple II HGR
byte order use opposite horizontal bit ordering.

## UI Helpers Built on the Font Renderer

`src/ui.c` builds several helpers on top of `print()`:

- `print_right_aligned()`
- `print_int_right_aligned()`
- `print_int_right_aligned_currency()`
- `print_int()`

These all currently render through the regular HGR text blitter. `print_bold()`
is a separate helper with no bold-specific alignment or numeric wrappers yet.

Example:

```c
print(10, 20, "Welcome to the World of Taipan!");
print_bold(10, 0, "Warehouse");
print_right_aligned(39, 0, "Turn:");
print_int_right_aligned_currency(39, 1, 2210000UL);
```

## Adapting the Pipeline

If you change either source font, update:

- source cell dimensions
- inter-cell border size
- grid dimensions
- ASCII mapping order
- downscaling logic if the source is no longer a clean 2:1 match
- extraction script assumptions
- the generated table in `include/font.h`

For this font, `7x8` remains the correct target because the runtime renderer is
hard-wired to one HGR byte per character column and eight HGR scanlines per text
row.

## Files

- `assets/img/taipan-font.png` - regular source image
- `assets/img/taipan-font-bold.png` - bold source image
- `tools/extract_font_bold.py` - bold conversion script
- `include/font.h` - generated glyph tables and font constants
- `asm/text_hgr.s` - runtime HGR text blitters
- `src/ui.c` - UI helpers that call the blitters
