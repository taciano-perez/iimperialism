#!/usr/bin/env python3
"""Export the in-game bitmap fonts into website-friendly assets.

This tool reads the checked-in 7x8 glyph tables from include/font.h and writes:
- BDF bitmap fonts for the regular and bold faces
- PNG sprite sheets for quick visual inspection and website fallbacks
- an HTML specimen page
- optional TTF/WOFF2 outline fonts when fontTools is installed

The generated TTF/WOFF2 fonts preserve the original bitmap appearance by
turning each lit pixel into a filled square in the glyph outline.
"""

from __future__ import annotations

import argparse
import html
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image

try:
    from fontTools.fontBuilder import FontBuilder
    from fontTools.pens.ttGlyphPen import TTGlyphPen
except ImportError:
    FontBuilder = None
    TTGlyphPen = None


FONT_FIRST_CHAR = 32
FONT_LAST_CHAR = 127
GLYPH_COUNT = FONT_LAST_CHAR - FONT_FIRST_CHAR + 1
GLYPH_WIDTH = 7
GLYPH_HEIGHT = 8
DEFAULT_ADVANCE_WIDTH = 8
DEFAULT_OUTPUT = Path("build/webfonts")
PNG_GLYPHS_PER_ROW = 16
PNG_PADDING = 1


@dataclass(frozen=True)
class BitmapFont:
    symbol: str
    display_name: str
    weight: int
    glyph_rows: list[list[int]]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Export the IImperialism bitmap fonts for web use."
    )
    parser.add_argument(
        "--header",
        default="include/font.h",
        help="Path to include/font.h (default: include/font.h)",
    )
    parser.add_argument(
        "--output-dir",
        default=str(DEFAULT_OUTPUT),
        help=f"Directory for generated assets (default: {DEFAULT_OUTPUT})",
    )
    parser.add_argument(
        "--family-name",
        default="IImperialism Taipan",
        help="Font family name used in generated metadata",
    )
    parser.add_argument(
        "--pixel-size",
        type=int,
        default=128,
        help="UPM and glyph pixel scale for generated TTF/WOFF2 output",
    )
    parser.add_argument(
        "--ttf",
        action="store_true",
        help="Generate TTF files when fontTools is installed",
    )
    parser.add_argument(
        "--woff2",
        action="store_true",
        help="Generate WOFF2 files when fontTools and a Brotli-backed WOFF2 encoder are installed",
    )
    return parser.parse_args()


def extract_font_arrays(header_text: str) -> dict[str, list[list[int]]]:
    arrays: dict[str, list[list[int]]] = {}
    current_symbol: str | None = None
    current_rows: list[list[int]] = []
    decl_pattern = re.compile(
        r"const\s+unsigned\s+char\s+(\w+)\s*\[\s*96\s*\]\s*\[\s*8\s*\]\s*=\s*\{"
    )
    row_pattern = re.compile(r"^\s*\{([^}]*)\},")

    for line in header_text.splitlines():
        if current_symbol is None:
            match = decl_pattern.match(line)
            if match:
                current_symbol = match.group(1)
                current_rows = []
            continue

        if line.strip() == "};":
            if len(current_rows) != GLYPH_COUNT:
                raise ValueError(
                    f"{current_symbol} contains {len(current_rows)} glyphs, expected {GLYPH_COUNT}"
                )
            arrays[current_symbol] = current_rows
            current_symbol = None
            current_rows = []
            continue

        row_match = row_pattern.match(line)
        if row_match is None:
            continue

        values = [part.strip() for part in row_match.group(1).split(",") if part.strip()]
        if len(values) != GLYPH_HEIGHT:
            raise ValueError(f"{current_symbol} contains a row with {len(values)} values")
        current_rows.append([int(value, 16) for value in values])

    if "font_data" not in arrays or "font_bold_data" not in arrays:
        raise ValueError("Failed to locate both font_data and font_bold_data in the header")
    return arrays


def bdf_name(text: str) -> str:
    return text.replace(" ", "-")


def glyph_comment(codepoint: int) -> str:
    if codepoint == 32:
        return "SPACE"
    if codepoint == 92:
        return "BACKSLASH"
    if codepoint == 127:
        return "FULL_BLOCK"
    return chr(codepoint)


def iter_glyph_pixels(rows: Iterable[int]) -> Iterable[tuple[int, int]]:
    row_list = list(rows)
    for y, row_value in enumerate(row_list):
        for x in range(GLYPH_WIDTH):
            bit = 1 << (GLYPH_WIDTH - 1 - x)
            if row_value & bit:
                yield x, y


def write_bdf(font: BitmapFont, output_path: Path, family_name: str) -> None:
    font_name = f"{family_name} {font.display_name}"
    lines = [
        "STARTFONT 2.1",
        f"FONT {bdf_name(font_name)}",
        f"SIZE {GLYPH_HEIGHT} 75 75",
        f"FONTBOUNDINGBOX {GLYPH_WIDTH} {GLYPH_HEIGHT} 0 0",
        "STARTPROPERTIES 5",
        f"FONT_ASCENT {GLYPH_HEIGHT}",
        "FONT_DESCENT 0",
        f"FAMILY_NAME \"{font_name}\"",
        f"WEIGHT_NAME \"{'Bold' if font.weight >= 700 else 'Regular'}\"",
        'SPACING "M"',
        "ENDPROPERTIES",
        f"CHARS {GLYPH_COUNT}",
    ]

    for offset, rows in enumerate(font.glyph_rows):
        codepoint = FONT_FIRST_CHAR + offset
        lines.extend(
            [
                f"STARTCHAR {glyph_comment(codepoint)}",
                f"ENCODING {codepoint}",
                "SWIDTH 500 0",
                f"DWIDTH {DEFAULT_ADVANCE_WIDTH} 0",
                f"BBX {GLYPH_WIDTH} {GLYPH_HEIGHT} 0 0",
                "BITMAP",
            ]
        )
        lines.extend(f"{row_value:02X}" for row_value in rows)
        lines.append("ENDCHAR")

    lines.append("ENDFONT")
    output_path.write_text("\n".join(lines) + "\n", encoding="ascii")


def write_sprite_sheet(font: BitmapFont, output_path: Path) -> None:
    rows = (GLYPH_COUNT + PNG_GLYPHS_PER_ROW - 1) // PNG_GLYPHS_PER_ROW
    cell_width = GLYPH_WIDTH + PNG_PADDING * 2
    cell_height = GLYPH_HEIGHT + PNG_PADDING * 2
    image = Image.new("RGBA", (PNG_GLYPHS_PER_ROW * cell_width, rows * cell_height), (0, 0, 0, 0))

    for index, glyph_rows in enumerate(font.glyph_rows):
        cell_x = index % PNG_GLYPHS_PER_ROW
        cell_y = index // PNG_GLYPHS_PER_ROW
        base_x = cell_x * cell_width + PNG_PADDING
        base_y = cell_y * cell_height + PNG_PADDING
        for pixel_x, pixel_y in iter_glyph_pixels(glyph_rows):
            image.putpixel((base_x + pixel_x, base_y + pixel_y), (255, 255, 255, 255))

    image.save(output_path)


def _draw_pixel_square(pen: TTGlyphPen, x: int, y: int, pixel_size: int) -> None:
    x0 = x * pixel_size
    x1 = x0 + pixel_size
    y_top = GLYPH_HEIGHT * pixel_size - y * pixel_size
    y_bottom = y_top - pixel_size
    pen.moveTo((x0, y_bottom))
    pen.lineTo((x1, y_bottom))
    pen.lineTo((x1, y_top))
    pen.lineTo((x0, y_top))
    pen.closePath()


def build_bitmap_ttf(font: BitmapFont, family_name: str, pixel_size: int) -> FontBuilder:
    if FontBuilder is None or TTGlyphPen is None:
        raise RuntimeError("fontTools is required for TTF/WOFF2 export")

    units_per_em = GLYPH_HEIGHT * pixel_size
    glyph_order = [".notdef"] + [f"uni{codepoint:04X}" for codepoint in range(FONT_FIRST_CHAR, FONT_LAST_CHAR + 1)]
    fb = FontBuilder(units_per_em, isTTF=True)
    fb.setupGlyphOrder(glyph_order)

    cmap = {
        codepoint: glyph_order[index + 1]
        for index, codepoint in enumerate(range(FONT_FIRST_CHAR, FONT_LAST_CHAR + 1))
    }
    fb.setupCharacterMap(cmap)

    advance_width = DEFAULT_ADVANCE_WIDTH * pixel_size
    glyphs = {}
    metrics = {}

    notdef_pen = TTGlyphPen(None)
    glyphs[".notdef"] = notdef_pen.glyph()
    metrics[".notdef"] = (advance_width, 0)

    for offset, glyph_rows in enumerate(font.glyph_rows):
        glyph_name = glyph_order[offset + 1]
        pen = TTGlyphPen(None)
        for pixel_x, pixel_y in iter_glyph_pixels(glyph_rows):
            _draw_pixel_square(pen, pixel_x, pixel_y, pixel_size)
        glyphs[glyph_name] = pen.glyph()
        metrics[glyph_name] = (advance_width, 0)

    fb.setupGlyf(glyphs)
    fb.setupHorizontalMetrics(metrics)
    fb.setupHorizontalHeader(
        ascent=GLYPH_HEIGHT * pixel_size,
        descent=0,
    )
    fb.setupNameTable(
        {
            "familyName": family_name,
            "styleName": font.display_name,
            "fullName": f"{family_name} {font.display_name}",
            "psName": f"{family_name.replace(' ', '')}-{font.display_name}",
            "uniqueFontIdentifier": f"{family_name} {font.display_name}",
            "version": "Version 1.0",
        }
    )
    fb.setupOS2(
        sTypoAscender=GLYPH_HEIGHT * pixel_size,
        sTypoDescender=0,
        usWinAscent=GLYPH_HEIGHT * pixel_size,
        usWinDescent=0,
        sxHeight=GLYPH_HEIGHT * pixel_size,
        sCapHeight=GLYPH_HEIGHT * pixel_size,
        usWeightClass=font.weight,
    )
    fb.setupPost()
    fb.setupMaxp()
    return fb


def write_ttf(font: BitmapFont, output_path: Path, family_name: str, pixel_size: int) -> None:
    fb = build_bitmap_ttf(font, family_name, pixel_size)
    fb.save(str(output_path))


def write_specimen(
    output_path: Path,
    family_name: str,
    regular_ttf: Path | None,
    bold_ttf: Path | None,
) -> None:
    regular_face = ""
    bold_face = ""
    if regular_ttf is not None:
        regular_face = (
            "@font-face {"
            f"font-family: '{family_name}';"
            f"src: url('{regular_ttf.name}') format('truetype');"
            "font-weight: 400;"
            "font-style: normal;"
            "}"
        )
    if bold_ttf is not None:
        bold_face = (
            "@font-face {"
            f"font-family: '{family_name}';"
            f"src: url('{bold_ttf.name}') format('truetype');"
            "font-weight: 700;"
            "font-style: normal;"
            "}"
        )

    specimen_text = html.escape(
        'THE COUNCIL OF NATIONS\n'
        '0123456789  $12345  +-*/\n'
        'ABCDEFGHIJKLMNOPQRSTUVWXYZ\n'
        'abcdefghijklmnopqrstuvwxyz\n'
        '! " # $ % & \' ( ) * + , - . /\n'
        '@ [ \\ ] ^ _ ` { | } ~'
    ).replace("\n", "<br>\n")

    output_path.write_text(
        "\n".join(
            [
                "<!doctype html>",
                "<html lang=\"en\">",
                "<head>",
                "  <meta charset=\"utf-8\">",
                "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">",
                "  <title>IImperialism Font Specimen</title>",
                "  <style>",
                f"    {regular_face}",
                f"    {bold_face}",
                "    :root { color-scheme: dark; }",
                "    body { margin: 0; padding: 32px; background: #102027; color: #f2e8c9; }",
                "    h1, p, pre { margin: 0 0 20px; }",
                "    .specimen { font-family: '" + family_name + "', monospace; font-size: 32px; line-height: 1.25; }",
                "    .bold { font-weight: 700; }",
                "  </style>",
                "</head>",
                "<body>",
                "  <h1>IImperialism Font Specimen</h1>",
                "  <p>TTF files are optional output. The BDF and PNG files are always generated.</p>",
                f"  <pre class=\"specimen\">{specimen_text}</pre>",
                f"  <pre class=\"specimen bold\">{specimen_text}</pre>",
                "</body>",
                "</html>",
                "",
            ]
        ),
        encoding="utf-8",
    )


def write_css(
    output_path: Path,
    family_name: str,
    regular_source: Path | None,
    bold_source: Path | None,
) -> None:
    lines = [
        "/* Generated by tools/export_web_fonts.py */",
    ]
    if regular_source is not None:
        regular_format = "woff2" if regular_source.suffix == ".woff2" else "truetype"
        lines.extend(
            [
                "@font-face {",
                f"  font-family: '{family_name}';",
                f"  src: url('{regular_source.name}') format('{regular_format}');",
                "  font-style: normal;",
                "  font-weight: 400;",
                "}",
            ]
        )
    if bold_source is not None:
        bold_format = "woff2" if bold_source.suffix == ".woff2" else "truetype"
        lines.extend(
            [
                "@font-face {",
                f"  font-family: '{family_name}';",
                f"  src: url('{bold_source.name}') format('{bold_format}');",
                "  font-style: normal;",
                "  font-weight: 700;",
                "}",
            ]
        )
    if regular_source is None and bold_source is None:
        lines.append("/* No TTF/WOFF2 assets were generated in this run. */")

    lines.extend(
        [
            "",
            ".iimperialism-font {",
            f"  font-family: '{family_name}', monospace;",
            "  font-smooth: never;",
            "  -webkit-font-smoothing: none;",
            "  text-rendering: optimizeSpeed;",
            "}",
            "",
        ]
    )
    output_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    header_path = Path(args.header)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    arrays = extract_font_arrays(header_path.read_text(encoding="utf-8"))
    fonts = [
        BitmapFont("font_data", "Regular", 400, arrays["font_data"]),
        BitmapFont("font_bold_data", "Bold", 700, arrays["font_bold_data"]),
    ]

    regular_ttf: Path | None = None
    bold_ttf: Path | None = None
    regular_css_source: Path | None = None
    bold_css_source: Path | None = None

    for font in fonts:
        stem = f"iimperialism-taipan-{font.display_name.lower()}"
        write_bdf(font, output_dir / f"{stem}.bdf", args.family_name)
        write_sprite_sheet(font, output_dir / f"{stem}.png")

        if args.ttf or args.woff2:
            if FontBuilder is None or TTGlyphPen is None:
                raise RuntimeError(
                    "fontTools is not installed. Install fonttools to enable TTF/WOFF2 export."
                )
            ttf_path = output_dir / f"{stem}.ttf"
            write_ttf(font, ttf_path, args.family_name, args.pixel_size)
            if font.weight >= 700:
                bold_ttf = ttf_path
                bold_css_source = ttf_path
            else:
                regular_ttf = ttf_path
                regular_css_source = ttf_path

            if args.woff2:
                try:
                    woff2_path = output_dir / f"{stem}.woff2"
                    woff2_font = build_bitmap_ttf(font, args.family_name, args.pixel_size)
                    woff2_font.font.flavor = "woff2"
                    woff2_font.save(str(woff2_path))
                except Exception as exc:
                    raise RuntimeError(
                        "WOFF2 export failed. Ensure fontTools has WOFF2 encoder support."
                    ) from exc
                if font.weight >= 700:
                    bold_css_source = woff2_path
                else:
                    regular_css_source = woff2_path

    write_css(output_dir / "fonts.css", args.family_name, regular_css_source, bold_css_source)
    write_specimen(output_dir / "specimen.html", args.family_name, regular_ttf, bold_ttf)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
