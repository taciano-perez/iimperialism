param(
    [Parameter(Mandatory = $true)]
    [string]$BuildDir
)

$files = Get-ChildItem $BuildDir -File -Filter '*.bin' | Sort-Object Name
if (-not $files) {
    Write-Host "No overlay binaries found in $BuildDir."
    exit 0
}

$rows = foreach ($f in $files) {
    $bytes = [System.IO.File]::ReadAllBytes($f.FullName)
    $lastNonZero = -1
    for ($i = $bytes.Length - 1; $i -ge 0; $i--) {
        if ($bytes[$i] -ne 0) {
            $lastNonZero = $i
            break
        }
    }

    $used = if ($lastNonZero -ge 0) { $lastNonZero + 1 } else { 0 }
    $pad = $bytes.Length - $used

    [PSCustomObject]@{
        Overlay            = $f.Name
        TotalBytes         = $bytes.Length
        UsedBytesApprox    = $used
        PaddingBytesApprox = $pad
        UsedPercentApprox  = [math]::Round((($used / $bytes.Length) * 100), 2)
    }
}

$rows | Format-Table -AutoSize
