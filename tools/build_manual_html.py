#!/usr/bin/env python3
"""Build the IImperialism manual HTML from the Markdown source.

This is a small purpose-built renderer for the manual's current Markdown subset:
- ATX headings (#, ##)
- paragraphs
- unordered lists
- ordered lists
- images
- inline code

The output is a styled standalone HTML document that loads the generated game
web fonts for headings and captions.
"""

from __future__ import annotations

import argparse
import html
import re
from pathlib import Path


IMAGE_RE = re.compile(r"^!\[([^\]]*)\]\(([^)]+)\)\s*$")
ORDERED_RE = re.compile(r"^(\d+)\.\s+(.*)$")
UNORDERED_RE = re.compile(r"^-\s+(.*)$")
CODE_RE = re.compile(r"`([^`]+)`")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build the manual HTML from docs/manual/iimperialism-manual.md."
    )
    parser.add_argument(
        "--input",
        default="docs/manual/iimperialism-manual.md",
        help="Input Markdown path (default: docs/manual/iimperialism-manual.md)",
    )
    parser.add_argument(
        "--output",
        default="docs/manual/iimperialism-manual.html",
        help="Output HTML path (default: docs/manual/iimperialism-manual.html)",
    )
    return parser.parse_args()


def slugify(text: str) -> str:
    slug = re.sub(r"[^a-z0-9]+", "-", text.lower()).strip("-")
    return slug or "section"


def render_inline(text: str) -> str:
    rendered = html.escape(text)
    rendered = rendered.replace("&lt;b&gt;", "<b>").replace("&lt;/b&gt;", "</b>")
    rendered = CODE_RE.sub(lambda m: f"<code>{html.escape(m.group(1))}</code>", rendered)
    return rendered


def flush_paragraph(buffer: list[str], out: list[str]) -> None:
    if not buffer:
        return
    text = " ".join(part.strip() for part in buffer).strip()
    if text:
        out.append(f"<p>{render_inline(text)}</p>")
    buffer.clear()


def flush_list(kind: str | None, items: list[str], out: list[str]) -> None:
    if kind is None or not items:
        return
    tag = "ol" if kind == "ol" else "ul"
    out.append(f"<{tag}>")
    for item in items:
        out.append(f"  <li>{render_inline(item)}</li>")
    out.append(f"</{tag}>")
    items.clear()


def build_toc(headings: list[tuple[str, str]]) -> str:
    if not headings:
        return ""
    items = "\n".join(
        f'  <li><a href="#{slug}">{render_inline(heading)}</a></li>'
        for heading, slug in headings
    )
    return (
        '<section class="manual-toc-page">'
        '<nav class="manual-toc" aria-labelledby="manual-toc-title">'
        '<h2 id="manual-toc-title">Contents</h2>'
        f"<ol>\n{items}\n</ol>"
        "</nav>"
        "</section>"
    )


def render_markdown(text: str) -> tuple[str, str, str, str]:
    title = "IImperialism! Manual"
    version = ""
    author = ""
    lines = text.splitlines()
    out: list[str] = []
    paragraph: list[str] = []
    list_kind: str | None = None
    list_items: list[str] = []
    headings: list[tuple[str, str]] = []
    in_front_matter = True

    for line in lines:
        stripped = line.strip()

        if in_front_matter and stripped.startswith("Version:"):
            version = stripped.split(":", 1)[1].strip()
            continue

        if in_front_matter and stripped.startswith("Author:"):
            author = stripped.split(":", 1)[1].strip()
            continue

        if not stripped:
            if in_front_matter:
                continue
            flush_paragraph(paragraph, out)
            flush_list(list_kind, list_items, out)
            list_kind = None
            continue

        if stripped.startswith("# "):
            flush_paragraph(paragraph, out)
            flush_list(list_kind, list_items, out)
            list_kind = None
            heading = stripped[2:].strip()
            title = heading
            continue

        if stripped.startswith("## "):
            in_front_matter = False
            flush_paragraph(paragraph, out)
            flush_list(list_kind, list_items, out)
            list_kind = None
            heading = stripped[3:].strip()
            slug = slugify(heading)
            headings.append((heading, slug))
            out.append(
                f"<section class=\"manual-section\" id=\"{slug}\">"
                f"<h2>{render_inline(heading)}</h2>"
            )
            continue

        image_match = IMAGE_RE.match(stripped)
        if image_match:
            flush_paragraph(paragraph, out)
            flush_list(list_kind, list_items, out)
            list_kind = None
            alt_text = image_match.group(1)
            src = image_match.group(2)
            out.append(
                "<figure class=\"manual-figure\">"
                f"<img src=\"{html.escape(src)}\" alt=\"{html.escape(alt_text)}\">"
                f"<figcaption>{render_inline(alt_text)}</figcaption>"
                "</figure>"
            )
            continue

        ordered_match = ORDERED_RE.match(stripped)
        if ordered_match:
            flush_paragraph(paragraph, out)
            if list_kind not in (None, "ol"):
                flush_list(list_kind, list_items, out)
            list_kind = "ol"
            list_items.append(ordered_match.group(2).strip())
            continue

        unordered_match = UNORDERED_RE.match(stripped)
        if unordered_match:
            flush_paragraph(paragraph, out)
            if list_kind not in (None, "ul"):
                flush_list(list_kind, list_items, out)
            list_kind = "ul"
            list_items.append(unordered_match.group(1).strip())
            continue

        paragraph.append(stripped)

    flush_paragraph(paragraph, out)
    flush_list(list_kind, list_items, out)

    body = "\n".join(out)
    # Close each manual section opened by h2.
    body = body.replace("<section class=\"manual-section\"", "\n<section class=\"manual-section\"")
    body = re.sub(r"(<section class=\"manual-section\".*?<h2>.*?</h2>)", r"\1", body, flags=re.DOTALL)

    # Balance sections by wrapping content between h2 blocks.
    parts = body.split("\n<section class=\"manual-section\"")
    rebuilt: list[str] = [parts[0]]
    for part in parts[1:]:
        rebuilt.append("<section class=\"manual-section\"" + part + "\n</section>")
    return title, version, author, build_toc(headings) + "\n" + "\n".join(rebuilt)


def build_cover(title: str, version: str, author: str) -> str:
    version_html = f'<p class="cover-meta">Version {render_inline(version)}</p>' if version else ""
    author_html = f'<p class="cover-meta">By {render_inline(author)}</p>' if author else ""
    return (
        '<section class="cover-page">'
        f'<h1 class="cover-title">{render_inline(title)}</h1>'
        f"{version_html}"
        f"{author_html}"
        "</section>"
    )


def build_document(title: str, version: str, author: str, body_html: str) -> str:
    return f"""<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>{html.escape(title)}</title>
    <link rel="stylesheet" href="../../assets/fonts/fonts.css">
    <style>
      :root {{
        --paper-bg: #e8d9a6;
        --paper: #f6eed0;
        --ink: #1a241d;
        --muted: #465448;
        --panel: #fcf6e3;
        --rule: #cfbd81;
        --accent: #b78b34;
      }}

      * {{
        box-sizing: border-box;
      }}

      body {{
        margin: 0;
        background: linear-gradient(180deg, #f0e3b7, var(--paper-bg));
        color: var(--ink);
        font-family: Garamond, "Adobe Garamond Pro", "Palatino Linotype", serif;
      }}

      .page {{
        width: min(920px, calc(100% - 32px));
        margin: 24px auto;
        padding: 36px 42px 44px;
        background: var(--paper);
        border: 1px solid rgba(183, 139, 52, 0.35);
        box-shadow: 0 24px 60px rgba(0, 0, 0, 0.28);
      }}

      .manual-header {{
        border-bottom: 3px solid var(--accent);
        margin-bottom: 28px;
        padding-bottom: 18px;
      }}

      .cover-page {{
        min-height: 220mm;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        text-align: center;
        page-break-after: always;
      }}

      .cover-title {{
        margin: 0 0 26px;
        font-size: 44px;
        line-height: 1.15;
        color: #7b5a16;
      }}

      .cover-meta {{
        margin: 8px 0 0;
        font-size: 24px;
        line-height: 1.4;
      }}

      .manual-toc {{
        margin: 0 0 30px;
        padding: 18px 20px 16px;
        background: var(--panel);
        border: 1px solid var(--rule);
      }}

      h1, h2, figcaption {{
        font-family: "IImperialism Taipan", monospace;
      }}

      h1 {{
        margin: 0;
        font-size: 38px;
        line-height: 1.1;
        color: #6f4f11;
      }}

      h2 {{
        margin: 0 0 16px;
        font-size: 22px;
        color: #8a651a;
      }}

      .manual-toc h2 {{
        margin-bottom: 14px;
      }}

      .manual-toc-page {{
        page-break-after: always;
      }}

      .manual-section + .manual-section {{
        margin-top: 28px;
      }}

      .manual-section {{
        padding-top: 8px;
      }}

      p, li {{
        font-size: 17px;
        line-height: 1.65;
      }}

      p {{
        margin: 0 0 14px;
      }}

      ul, ol {{
        margin: 0 0 16px 22px;
        padding: 0;
      }}

      .manual-toc ol {{
        columns: 2;
        column-gap: 28px;
        margin-bottom: 0;
      }}

      li + li {{
        margin-top: 6px;
      }}

      .manual-toc a {{
        color: inherit;
        text-decoration: none;
      }}

      code {{
        padding: 1px 5px;
        background: rgba(183, 139, 52, 0.1);
        border-radius: 4px;
        font-family: Consolas, "Courier New", monospace;
        font-size: 0.92em;
      }}

      .manual-figure {{
        margin: 18px 0 20px;
        padding: 14px;
        background: var(--panel);
        border: 1px solid var(--rule);
      }}

      .manual-figure img {{
        display: block;
        width: 280px;
        max-width: 100%;
        margin: 0 auto;
        height: auto;
        image-rendering: pixelated;
        background: #000;
      }}

      .manual-figure figcaption {{
        margin-top: 10px;
        font-size: 14px;
        color: var(--muted);
      }}

      @page {{
        size: A4;
        margin: 18mm 14mm 20mm;
      }}

      @media print {{
        body {{
          background: var(--paper);
          -webkit-print-color-adjust: exact;
          print-color-adjust: exact;
        }}

        .page {{
          width: auto;
          margin: 0;
          padding: 0;
          border: 0;
          box-shadow: none;
          background: var(--paper);
        }}

        .manual-figure {{
          break-inside: avoid;
        }}

        h2 {{
          break-after: avoid;
        }}
      }}

      @media (max-width: 700px) {{
        .page {{
          width: min(100% - 20px, 920px);
          padding: 24px 18px 30px;
        }}

        h1 {{
          font-size: 30px;
        }}

        .cover-title {{
          font-size: 34px;
        }}

        .cover-meta {{
          font-size: 20px;
        }}

        .manual-toc ol {{
          columns: 1;
        }}

        .manual-figure img {{
          max-width: 100%;
        }}
      }}
    </style>
  </head>
  <body>
    <main class="page">
{build_cover(title, version, author)}
{body_html}
    </main>
  </body>
</html>
"""


def main() -> int:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)
    title, version, author, body_html = render_markdown(input_path.read_text(encoding="utf-8"))
    output_path.write_text(build_document(title, version, author, body_html), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
