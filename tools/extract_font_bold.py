#!/usr/bin/env python3
"""Extract the bold Taipan bitmap font into a C glyph table.

The source image is expected to use:
- 14x16 source glyphs
- a 2-pixel grid between glyphs
- a matching 2-pixel outer border
- 32 glyphs per row
- ASCII ordering starting at character 32

The runtime target format is the existing Apple II font table layout:
- 96 glyphs (ASCII 32..127)
- each glyph reduced to 7x8 with a 2:1 any-pixel-on rule
- each 7-pixel row packed into one byte, bit 6 = leftmost pixel
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterable

from PIL import Image

FONT_FIRST_CHAR = 32
FONT_LAST_CHAR = 127
GLYPH_COUNT = FONT_LAST_CHAR - FONT_FIRST_CHAR + 1

SOURCE_GLYPH_WIDTH = 14
SOURCE_GLYPH_HEIGHT = 16
TARGET_GLYPH_WIDTH = 7
TARGET_GLYPH_HEIGHT = 8

GRID_WIDTH = 2
GRID_HEIGHT = 2
OUTER_BORDER_X = 2
OUTER_BORDER_Y = 2
GLYPHS_PER_ROW = 32
ROWS_TO_EXTRACT = 3

DEFAULT_SOURCE = Path("assets/img/taipan-font-bold.png")
DEFAULT_SYMBOL = "font_bold_data"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract assets/img/taipan-font-bold.png into a C glyph table."
    )
    parser.add_argument(
        "source",
        nargs="?",
        default=str(DEFAULT_SOURCE),
        help=f"Path to the source PNG (default: {DEFAULT_SOURCE})",
    )
    parser.add_argument(
        "--symbol",
        default=DEFAULT_SYMBOL,
        help=f"C symbol name for the emitted table (default: {DEFAULT_SYMBOL})",
    )
    return parser.parse_args()


def load_image(path: Path) -> Image.Image:
    image = Image.open(path).convert("L")
    min_width = OUTER_BORDER_X + GLYPHS_PER_ROW * (SOURCE_GLYPH_WIDTH + GRID_WIDTH)
    min_height = OUTER_BORDER_Y + ROWS_TO_EXTRACT * (
        SOURCE_GLYPH_HEIGHT + GRID_HEIGHT
    )
    if image.size[0] != min_width:
        raise ValueError(
            f"Unexpected image width {image.size[0]}; expected {min_width} for {path}"
        )
    if image.size[1] < min_height:
        raise ValueError(
            f"Unexpected image height {image.size[1]}; expected at least {min_height} for {path}"
        )
    return image


def source_origin(index: int) -> tuple[int, int]:
    col = index % GLYPHS_PER_ROW
    row = index // GLYPHS_PER_ROW
    x_start = OUTER_BORDER_X + col * (SOURCE_GLYPH_WIDTH + GRID_WIDTH)
    y_start = OUTER_BORDER_Y + row * (SOURCE_GLYPH_HEIGHT + GRID_HEIGHT)
    return x_start, y_start


def source_block_has_ink(image: Image.Image, x_start: int, y_start: int) -> bool:
    for y in range(y_start, y_start + 2):
        for x in range(x_start, x_start + 2):
            if image.getpixel((x, y)) > 0:
                return True
    return False


def pack_target_row(bits: Iterable[bool]) -> int:
    value = 0
    for x, bit_on in enumerate(bits):
        if bit_on:
            value |= 1 << (TARGET_GLYPH_WIDTH - 1 - x)
    return value


def extract_glyph(image: Image.Image, index: int) -> list[int]:
    x_start, y_start = source_origin(index)
    rows: list[int] = []

    for target_y in range(TARGET_GLYPH_HEIGHT):
        row_bits = []
        for target_x in range(TARGET_GLYPH_WIDTH):
            block_x = x_start + target_x * 2
            block_y = y_start + target_y * 2
            row_bits.append(source_block_has_ink(image, block_x, block_y))
        rows.append(pack_target_row(row_bits))

    return rows


def glyph_comment(index: int) -> str:
    codepoint = FONT_FIRST_CHAR + index
    if codepoint == 32:
        return " "
    if codepoint == 92:
        return "backslash"
    if codepoint == 127:
        return "(full block)"
    return chr(codepoint)


def emit_table(symbol: str, glyph_rows: list[list[int]]) -> str:
    lines = [
        f"const unsigned char {symbol}[{GLYPH_COUNT}][{TARGET_GLYPH_HEIGHT}] = {{"
    ]
    for index, rows in enumerate(glyph_rows):
        row_data = ", ".join(f"0x{row:02X}" for row in rows)
        lines.append(f"    {{{row_data}}}, // {glyph_comment(index)}")
    lines.append("};")
    return "\n".join(lines)


def main() -> int:
    args = parse_args()
    source_path = Path(args.source)
    image = load_image(source_path)
    glyph_rows = [extract_glyph(image, index) for index in range(GLYPH_COUNT)]
    print(emit_table(args.symbol, glyph_rows))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
