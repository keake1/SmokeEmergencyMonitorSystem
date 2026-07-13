param(
    [Parameter(Mandatory = $true)]
    [string]$Port,
    [string]$File = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
if (-not $File) {
    $File = Join-Path $root "build\SensorBoard.ihx"
}

if (-not (Test-Path $File)) {
    throw "Firmware not found: $File. Run .\tools\build.ps1 first."
}

python -m stcgal -p $Port -P auto $File
