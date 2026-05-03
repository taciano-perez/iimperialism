param(
    [string]$HtmlPath = "docs/manual/iimperialism-manual.html",
    [string]$PdfPath = "build/manual/iimperialism-manual.pdf",
    [string]$BrowserPath = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $repoRoot

try {
    python tools/build_manual_html.py --output $HtmlPath
    if ($BrowserPath) {
        python tools/build_manual_pdf.py --html $HtmlPath --pdf $PdfPath --browser $BrowserPath
    }
    else {
        python tools/build_manual_pdf.py --html $HtmlPath --pdf $PdfPath
    }
}
finally {
    Pop-Location
}
