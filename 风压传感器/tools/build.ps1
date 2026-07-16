param(
    [string]$Sdcc = "sdcc"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root "build"
$src = Join-Path $root "src"
$inc = Join-Path $root "include"

New-Item -ItemType Directory -Force -Path $build | Out-Null

$sdccCandidates = @(
    $Sdcc,
    "D:\Tools\SDCCMCS51\sdcc\bin\sdcc.exe",
    "D:\Tools\SDCC\bin\sdcc.exe",
    "D:\SDCC\bin\sdcc.exe",
    "C:\Program Files\SDCC\bin\sdcc.exe"
)

$sdccPath = $null
foreach ($candidate in $sdccCandidates) {
    $cmd = Get-Command $candidate -ErrorAction SilentlyContinue
    if ($cmd) {
        $sdccPath = $cmd.Source
        break
    }
    if (Test-Path $candidate) {
        $sdccPath = $candidate
        break
    }
}

if (-not $sdccPath) {
    throw "SDCC not found. Install SDCC for Windows, or pass -Sdcc C:\path\to\sdcc.exe"
}

$versionText = (& $sdccPath --version 2>&1) -join "`n"
if ($versionText -notmatch "mcs51") {
    throw "The SDCC found at '$sdccPath' does not support MCS-51. Install official SDCC for Windows from https://sdcc.sourceforge.net/."
}

$sources = @(
    "main.c",
    "board.c",
    "crc16.c",
    "display.c",
    "pressure.c",
    "sensor_modbus.c",
    "uart.c"
) | ForEach-Object { Join-Path $src $_ }

$objects = @()
foreach ($sourceFile in $sources) {
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($sourceFile)
    $relFile = Join-Path $build "$baseName.rel"
    & $sdccPath -mmcs51 --std-c99 --model-small --out-fmt-ihx "-I$inc" -c $sourceFile "-o$relFile"
    if ($LASTEXITCODE -ne 0) {
        throw "SDCC compile failed for $sourceFile with exit code $LASTEXITCODE"
    }
    $objects += $relFile
}

& $sdccPath -mmcs51 --std-c99 --model-small --out-fmt-ihx "-o$(Join-Path $build 'SensorBoard.ihx')" $objects
if ($LASTEXITCODE -ne 0) {
    throw "SDCC link failed with exit code $LASTEXITCODE"
}

$packihx = Join-Path (Split-Path $sdccPath -Parent) "packihx.exe"
if (Test-Path $packihx) {
    & $packihx (Join-Path $build "SensorBoard.ihx") | Set-Content -Encoding ascii (Join-Path $build "SensorBoard.hex")
    if ($LASTEXITCODE -ne 0) {
        throw "packihx failed with exit code $LASTEXITCODE"
    }
}

Write-Host "Built: $(Join-Path $build 'SensorBoard.ihx')"
