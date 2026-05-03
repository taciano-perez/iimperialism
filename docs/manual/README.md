# Manual Build

The manual source is:

- `docs/manual/iimperialism-manual.md`

The generated HTML output is:

- `docs/manual/iimperialism-manual.html`

The generated PDF output is:

- `build/manual/iimperialism-manual.pdf`

## Build HTML

From the repository root:

```bash
python tools/build_manual_html.py
```

This builds a styled HTML manual that uses:

- the game font for the title, section headings, and captions
- Garamond-style serif body text
- the screenshots from `assets/screenshots/`

## Build PDF

From the repository root on Windows:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build_manual_pdf.ps1
```

This command:

1. regenerates the HTML manual
2. renders the PDF through Playwright
3. uses a locally installed Microsoft Edge or Google Chrome executable

## Manual Layout

The current PDF layout is:

1. cover page
2. table of contents
3. `INTRODUCTION` begins on the third page

## Notes

- If the PDF build is unavailable, you can still generate the HTML and print it
  manually from a browser.
- If you need to point the PDF build at a specific browser executable, use:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build_manual_pdf.ps1 -BrowserPath "C:\Path\To\msedge.exe"
```

## Versioned Releases

If you want to build the disk image and the manual together into a versioned
release folder, use:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build_release.ps1
```

That flow updates the manual version, builds the PDF, and copies the release
artifacts into:

- `releases/<version>/`
