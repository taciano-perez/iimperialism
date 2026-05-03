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

The checked-in bold source currently contains blank glyphs for `|` and `}`.
Those blanks are reflected in the generated runtime table.

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

## Conversion And Export Scripts

The font-related scripts in this repo now cover two separate jobs:

1. extracting runtime glyph tables from source PNG sheets
2. exporting the checked-in runtime glyph tables as website assets

Current in-tree script:

- `tools/extract_font_bold.py`
- `tools/export_web_fonts.py`

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

### Web Font Export

The web-font export script works from the checked-in `include/font.h` glyph
tables instead of re-extracting from the PNG sources.

Default usage:

```bash
python tools/export_web_fonts.py
```

This writes output into `build/webfonts/` by default.

To write the website assets into the checked-in website path:

```bash
python tools/export_web_fonts.py --output-dir assets/fonts --ttf --woff2
```

The exporter can emit:

- BDF bitmap fonts for both faces
- PNG sprite sheets for inspection or sprite-based website fallbacks
- `fonts.css` with `@font-face` rules and a helper class
- `specimen.html` for visual inspection
- optional TTF files when `fonttools` is installed locally
- optional WOFF2 files when `fonttools` is installed locally and the Python
  `brotli` package is available

The generated `TTF` and `WOFF2` files keep the bitmap look by converting each
lit source pixel into a filled square in the glyph outline.

### Current Generated Website Assets

The repository currently ships these exported web-font assets in `assets/fonts/`:

- `iimperialism-taipan-regular.ttf`
- `iimperialism-taipan-regular.woff2`
- `iimperialism-taipan-bold.ttf`
- `iimperialism-taipan-bold.woff2`
- `fonts.css`
- `specimen.html`

The same export run also writes BDF and PNG versions of each face:

- `iimperialism-taipan-regular.bdf`
- `iimperialism-taipan-regular.png`
- `iimperialism-taipan-bold.bdf`
- `iimperialism-taipan-bold.png`

### Exporter Inputs And Limits

The exporter assumes:

- glyph coverage remains ASCII `32..127`
- glyph bitmaps remain `7x8`
- advance width remains one fixed 8-pixel cell
- the glyph tables in `include/font.h` remain the canonical source for website
  export

It does not derive kerning, extended Unicode coverage, or smooth outline curves.
The resulting web fonts are intentionally pixel-styled.

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

and

1. `print_inverted()` in `src/ui.c`
2. `draw_text_hgr_opaque_inverted()` in `asm/text_hgr.s`
3. direct HGR byte writes using inverted regular-font bits

Relevant entry point:

```c
void print(unsigned char x, unsigned char y, const char* text) {
    draw_text_hgr_opaque(text, x, y);
}

void print_inverted(unsigned char x, unsigned char y, const char* text) {
    draw_text_hgr_opaque_inverted(text, x, y);
}

void print_bold(unsigned char x, unsigned char y, const char* text) {
    draw_text_hgr_opaque_bold(text, x, y);
}
```

`src/ui.c` is currently linked into the `LC` segment in Language Card RAM, while
`asm/text_hgr.s` remains in main-RAM `LOWCODE`. The overlay-visible ABI is still
provided through the resident jump table, not by direct overlay-to-LC linkage.

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
- for `print_inverted()`, inverts only the 7 visible glyph bits so white glyph
  pixels become black and the background cell becomes white

The bit reversal step is required because the stored glyph rows and Apple II HGR
byte order use opposite horizontal bit ordering.

## UI Helpers Built on the Font Renderer

`src/ui.c` builds several helpers on top of `print()`:

- `print_inverted()`
- `print_right_aligned()`
- `print_int_right_aligned()`
- `print_int_right_aligned_currency()`
- `print_int()`

These all currently render through the regular HGR text blitter. `print_bold()`
is a separate helper with no bold-specific alignment or numeric wrappers yet.

Example:

```c
print(10, 20, "Welcome to the World of Taipan!");
print_inverted(10, 1, "Selected");
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
- `assets/fonts/` - exported website font assets and specimen files
- `tools/extract_font_bold.py` - bold conversion script
- `tools/export_web_fonts.py` - website font exporter from `include/font.h`
- `include/font.h` - generated glyph tables and font constants
- `asm/text_hgr.s` - runtime HGR text blitters
- `src/ui.c` - UI helpers in the `LC` segment that call the blitters
