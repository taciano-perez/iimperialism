param(
    [Parameter(Mandatory = $true)]
    [string]$BuildDir
)

function Format-HexAddr {
    param([int]$Value)
    return ('0x{0:X4}' -f $Value)
}

function New-UsageRow {
    param(
        [string]$Segment,
        [int]$Start,
        [int]$End,
        [int]$Used
    )

    $capacity = $End - $Start + 1
    $free = $capacity - $Used
    $usedPercent = if ($capacity -gt 0) {
        [math]::Round(($Used / $capacity) * 100, 2)
    } else {
        0
    }

    return [PSCustomObject]@{
        Segment      = $Segment
        Start        = Format-HexAddr $Start
        End          = Format-HexAddr $End
        Capacity     = $capacity
        Used         = $Used
        Free         = $free
        UsedPercent  = $usedPercent
    }
}

$mapPath = Join-Path $BuildDir 'iimperialism.map'
if (-not (Test-Path $mapPath)) {
    Write-Error "Map file not found: $mapPath"
    exit 1
}

$mapText = Get-Content $mapPath -Raw
$segmentMatches = [regex]::Matches(
    $mapText,
    '(?m)^(?<name>[A-Z_]+)\s+(?<start>[0-9A-F]{6})\s+(?<end>[0-9A-F]{6})\s+(?<size>[0-9A-F]{6})\s+[0-9A-F]{5}\s*$'
)

$segments = @{}
foreach ($match in $segmentMatches) {
    $segments[$match.Groups['name'].Value] = @{
        Start = [Convert]::ToInt32($match.Groups['start'].Value, 16)
        End = [Convert]::ToInt32($match.Groups['end'].Value, 16)
        Size = [Convert]::ToInt32($match.Groups['size'].Value, 16)
    }
}

$fixedRows = @()

if ($segments.ContainsKey('STARTUP')) {
    $seg = $segments['STARTUP']
    $fixedRows += New-UsageRow 'STARTUP' $seg.Start $seg.End $seg.Size
}

if ($segments.ContainsKey('JMPTAB')) {
    $seg = $segments['JMPTAB']
    $fixedRows += New-UsageRow 'JMPTAB' $seg.Start $seg.End $seg.Size
}

if ($segments.ContainsKey('LOWCODE')) {
    $seg = $segments['LOWCODE']
    $fixedRows += New-UsageRow 'LOWCODE' $seg.Start 0x1FFF $seg.Size
}

$highSegments = @('CODE', 'RODATA', 'DATA', 'INIT', 'ONCE', 'BSS')
$highUsed = 0
$highMaxEnd = 0x3FFF
foreach ($name in $highSegments) {
    if ($segments.ContainsKey($name)) {
        $highUsed += $segments[$name].Size
        if ($segments[$name].End -gt $highMaxEnd) {
            $highMaxEnd = $segments[$name].End
        }
    }
}
$residentSafeEnd = 0x87FF
$fixedRows += New-UsageRow 'RESIDENT_MAIN_SAFE' 0x4000 $residentSafeEnd ($highMaxEnd - 0x4000 + 1)
$fixedRows += New-UsageRow 'OVERLAY_SLOT' 0x8800 0x8FFF 0

$stackStart = 0x8E00
$stackEnd = 0x95FF
$fixedRows += New-UsageRow 'C_STACK' $stackStart $stackEnd ($stackEnd - $stackStart + 1)

$lcStart = 0xD400
$lcEnd = 0xDFFF
$lcUsed = if ($segments.ContainsKey('LC')) { $segments['LC'].Size } else { 0 }
$fixedRows += New-UsageRow 'LANGUAGE_CARD' $lcStart $lcEnd $lcUsed

Write-Host ''
Write-Host 'Resident Memory Usage'
$fixedRows | Format-Table -AutoSize

$detailRows = foreach ($name in @('CODE', 'RODATA', 'DATA', 'INIT', 'ONCE', 'BSS')) {
    if ($segments.ContainsKey($name)) {
        $seg = $segments[$name]
        [PSCustomObject]@{
            Segment = $name
            Start   = Format-HexAddr $seg.Start
            End     = Format-HexAddr $seg.End
            Size    = $seg.Size
        }
    }
}

if ($detailRows) {
    Write-Host ''
    Write-Host 'High Main Pool Breakdown'
    $detailRows | Format-Table -AutoSize
}

$overlapRows = @()
foreach ($name in $highSegments) {
    if ($segments.ContainsKey($name)) {
        $seg = $segments[$name]
        if ($seg.End -gt $residentSafeEnd) {
            $overlapRows += [PSCustomObject]@{
                Segment = $name
                Start   = Format-HexAddr $seg.Start
                End     = Format-HexAddr $seg.End
                Note    = 'Overlaps overlay-reserved RAM ($8800-$8FFF)'
            }
        }
    }
}

if ($overlapRows.Count -gt 0) {
    Write-Host ''
    Write-Host 'Resident / Overlay Overlap Warnings'
    $overlapRows | Format-Table -AutoSize
}

$overlayFiles = Get-ChildItem $BuildDir -File -Filter '*.bin' | Sort-Object Name
$overlayRows = foreach ($file in $overlayFiles) {
    $bytes = [System.IO.File]::ReadAllBytes($file.FullName)
    $lastNonZero = -1
    for ($i = $bytes.Length - 1; $i -ge 0; $i--) {
        if ($bytes[$i] -ne 0) {
            $lastNonZero = $i
            break
        }
    }

    $used = if ($lastNonZero -ge 0) { $lastNonZero + 1 } else { 0 }
    $capacity = $bytes.Length
    $free = $capacity - $used

    [PSCustomObject]@{
        Overlay     = $file.Name
        Capacity    = $capacity
        UsedApprox  = $used
        FreeApprox  = $free
        UsedPercent = [math]::Round((($used / $capacity) * 100), 2)
    }
}

if ($overlayRows) {
    Write-Host ''
    Write-Host 'Overlay Slot Usage'
    $overlayRows | Format-Table -AutoSize
}
