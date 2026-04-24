param(
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$thirdPartyDir = Join-Path $repoRoot 'third_party'

$sources = @(
    @{
        Name = 'acme'
        Url = 'https://github.com/meonwax/acme.git'
    }
    @{
        Name = 'qboot'
        Url = 'https://github.com/peterferrie/qboot.git'
    }
    @{
        Name = 'prorwts'
        Url = 'https://github.com/peterferrie/prorwts.git'
    }
)

function Require-Command {
    param([string]$Name)

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if (-not $command) {
        throw "Required command not found on PATH: $Name"
    }
    return $command.Source
}

function Clone-Source {
    param(
        [string]$Name,
        [string]$Url
    )

    $target = Join-Path $thirdPartyDir $Name
    if (Test-Path $target) {
        if (-not $Force) {
            throw "Target already exists: $target. Re-run with -Force to replace it."
        }
        Remove-Item -Recurse -Force $target
    }

    & git clone $Url $target
}

function Resolve-Make {
    foreach ($candidate in @('mingw32-make', 'make')) {
        $command = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($command) {
            return $command.Source
        }
    }
    throw "Could not find mingw32-make or make on PATH."
}

Require-Command 'git' | Out-Null

if (-not (Test-Path $thirdPartyDir)) {
    New-Item -ItemType Directory -Path $thirdPartyDir | Out-Null
}

foreach ($source in $sources) {
    Clone-Source -Name $source.Name -Url $source.Url
}

$makeExe = Resolve-Make
$acmeSrc = Join-Path $thirdPartyDir 'acme\src'
& $makeExe -f Makefile.mingw -C $acmeSrc

$acmeExe = Join-Path $acmeSrc 'acme.exe'
if (-not (Test-Path $acmeExe)) {
    throw "ACME build did not produce $acmeExe"
}

Write-Host "third_party rebuilt successfully."
Write-Host "ACME: $acmeExe"
