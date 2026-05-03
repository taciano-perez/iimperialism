#!/usr/bin/env python3
"""Build the IImperialism manual PDF using Playwright.

This uses Playwright's page.pdf() API so the PDF can have proper header/footer
templates and page numbers without overlapping the content area.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from playwright.sync_api import sync_playwright


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build docs/manual/iimperialism-manual.pdf from the HTML manual."
    )
    parser.add_argument(
        "--html",
        default="docs/manual/iimperialism-manual.html",
        help="Input HTML path (default: docs/manual/iimperialism-manual.html)",
    )
    parser.add_argument(
        "--pdf",
        default="build/manual/iimperialism-manual.pdf",
        help="Output PDF path (default: build/manual/iimperialism-manual.pdf)",
    )
    parser.add_argument(
        "--browser",
        default="",
        help="Optional browser executable path",
    )
    return parser.parse_args()


def detect_browser() -> Path | None:
    candidates = [
        Path.home() / "AppData/Local/Microsoft/Edge/Application/msedge.exe",
        Path("C:/Program Files/Microsoft/Edge/Application/msedge.exe"),
        Path("C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe"),
        Path("C:/Program Files/Google/Chrome/Application/chrome.exe"),
        Path("C:/Program Files (x86)/Google/Chrome/Application/chrome.exe"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return None


def build_header_template() -> str:
    return """
    <div style="width:100%; font-size:10px; color:#7f6020; padding:0 12mm 3mm; border-bottom:1px solid #cfbd81; font-family:'Courier New', monospace;">
      IImperialism! Manual
    </div>
    """


def build_footer_template() -> str:
    return """
    <div style="width:100%; font-size:10px; color:#7f6020; padding:3mm 12mm 0; border-top:1px solid #cfbd81; text-align:right; font-family:Garamond, 'Palatino Linotype', serif;">
      Page <span class="pageNumber"></span> of <span class="totalPages"></span>
    </div>
    """


def main() -> int:
    args = parse_args()
    html_path = Path(args.html).resolve()
    pdf_path = Path(args.pdf).resolve()
    pdf_path.parent.mkdir(parents=True, exist_ok=True)

    browser_path = Path(args.browser).resolve() if args.browser else detect_browser()
    if browser_path is None or not browser_path.exists():
        raise RuntimeError(
            "No supported Edge/Chrome browser found. Provide --browser with an executable path."
        )

    with sync_playwright() as playwright:
        browser = playwright.chromium.launch(
            executable_path=str(browser_path),
            headless=True,
            args=["--disable-gpu"],
        )
        page = browser.new_page()
        page.goto(html_path.as_uri(), wait_until="networkidle")
        page.pdf(
            path=str(pdf_path),
            format="A4",
            print_background=True,
            display_header_footer=True,
            header_template=build_header_template(),
            footer_template=build_footer_template(),
            margin={
                "top": "18mm",
                "right": "14mm",
                "bottom": "20mm",
                "left": "14mm",
            },
        )
        browser.close()

    if not pdf_path.exists():
        raise RuntimeError(f"PDF generation did not create '{pdf_path}'.")

    print(f"Wrote {pdf_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
