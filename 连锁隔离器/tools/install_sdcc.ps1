$ErrorActionPreference = "Stop"

$tools = "D:\Tools"
$installer = Join-Path $tools "sdcc-4.5.0-x64-setup.exe"
$url = "https://excellmedia.dl.sourceforge.net/project/sdcc/sdcc-win64/4.5.0/sdcc-4.5.0-x64-setup.exe"

New-Item -ItemType Directory -Force -Path $tools | Out-Null

if (-not (Test-Path $installer)) {
    Invoke-WebRequest -Uri $url -OutFile $installer
}

Write-Host "Downloaded: $installer"
Write-Host "Run this installer as administrator, or double-click it:"
Write-Host "  $installer"
Write-Host "Default install path is C:\Program Files\SDCC."
Write-Host "After installation, run:"
Write-Host "  .\tools\build.ps1"
