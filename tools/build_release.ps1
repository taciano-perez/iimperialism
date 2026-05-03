param(
    [string]$Version = "",
    [string]$VersionFile = "RELEASE_VERSION.txt",
    [switch]$PromptForVersion,
    [switch]$Force
)

$ErrorActionPreference = "Stop"

function Resolve-ReleaseVersion {
    param(
        [string]$ExplicitVersion,
        [string]$VersionFilePath,
        [bool]$Prompt
    )

    if ($ExplicitVersion) {
        return $ExplicitVersion.Trim()
    }

    if (-not $Prompt -and (Test-Path $VersionFilePath)) {
        $fileVersion = (Get-Content $VersionFilePath -Raw).Trim()
        if ($fileVersion) {
            return $fileVersion
        }
    }

    $inputVersion = (Read-Host "Release version").Trim()
    if (-not $inputVersion) {
        throw "No release version provided."
    }
    return $inputVersion
}

function Assert-VersionFormat {
    param([string]$Value)
    if ($Value -notmatch '^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$') {
        throw "Version '$Value' is not in a supported release format."
    }
}

function Update-ManualVersion {
    param(
        [string]$ManualPath,
        [string]$VersionValue
    )

    $manualText = Get-Content $ManualPath -Raw
    if ($manualText -match '(?m)^Version:\s+.+$') {
        $updated = [regex]::Replace($manualText, '(?m)^Version:\s+.+$', "Version: $VersionValue", 1)
    }
    else {
        throw "Could not find a 'Version:' line in $ManualPath."
    }
    Set-Content $ManualPath $updated
}

function Find-MakeExecutable {
    $candidates = @("make", "mingw32-make")
    foreach ($candidate in $candidates) {
        $command = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($command) {
            return $command.Source
        }
    }
    throw "Could not find 'make' or 'mingw32-make' on PATH."
}

$repoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $repoRoot

try {
    $resolvedVersionFile = Join-Path $repoRoot $VersionFile
    $releaseVersion = Resolve-ReleaseVersion -ExplicitVersion $Version -VersionFilePath $resolvedVersionFile -Prompt $PromptForVersion.IsPresent
    Assert-VersionFormat $releaseVersion

    if (-not (Test-Path (Split-Path -Parent $resolvedVersionFile))) {
        New-Item -ItemType Directory -Path (Split-Path -Parent $resolvedVersionFile) | Out-Null
    }
    Set-Content $resolvedVersionFile "$releaseVersion`n"

    $manualPath = Join-Path $repoRoot "docs/manual/iimperialism-manual.md"
    Update-ManualVersion -ManualPath $manualPath -VersionValue $releaseVersion

    $releaseDir = Join-Path $repoRoot "releases/$releaseVersion"
    if ((Test-Path $releaseDir) -and -not $Force) {
        throw "Release directory '$releaseDir' already exists. Use -Force to overwrite its artifacts."
    }
    New-Item -ItemType Directory -Force -Path $releaseDir | Out-Null

    $makeExe = Find-MakeExecutable
    & $makeExe "SHELL=cmd" "disk"

    powershell -ExecutionPolicy Bypass -File "tools/build_manual_pdf.ps1"

    $diskSource = Join-Path $repoRoot "assets/iimperialism.dsk"
    $manualPdfSource = Join-Path $repoRoot "build/manual/iimperialism-manual.pdf"
    $diskDest = Join-Path $releaseDir "iimperialism-$releaseVersion.dsk"
    $manualPdfDest = Join-Path $releaseDir "iimperialism-manual-$releaseVersion.pdf"

    if (-not (Test-Path $diskSource)) {
        throw "Expected disk image '$diskSource' was not created."
    }
    if (-not (Test-Path $manualPdfSource)) {
        throw "Expected manual PDF '$manualPdfSource' was not created."
    }

    Copy-Item $diskSource $diskDest -Force
    Copy-Item $manualPdfSource $manualPdfDest -Force

    Write-Host "Wrote release artifacts to $releaseDir"
}
finally {
    Pop-Location
}
