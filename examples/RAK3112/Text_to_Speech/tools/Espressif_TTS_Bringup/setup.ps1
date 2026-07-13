# Clone esp-skainet and build chinese_tts (requires ESP-IDF PowerShell environment).
$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot
$Skainet = Join-Path $Root "esp-skainet"
$Example = Join-Path $Skainet "examples\chinese_tts"

if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: idf.py not found. Open 'ESP-IDF PowerShell' from Start Menu, then run this script again."
    exit 1
}

if (-not (Test-Path $Skainet)) {
    Write-Host "Cloning esp-skainet..."
    git clone --depth 1 https://github.com/espressif/esp-skainet.git $Skainet
} else {
    Write-Host "esp-skainet already exists, skipping clone."
}

Set-Location $Example
Write-Host "Target: esp32s3"
idf.py set-target esp32s3
Write-Host "Building chinese_tts..."
idf.py build

Write-Host ""
Write-Host "Build OK. Next steps:"
Write-Host "  cd $Example"
Write-Host "  idf.py -p COMx flash monitor"
Write-Host "  Flash voice .dat with flash_voicedata.sh (see README.md in this folder)."
