# Flash Espressif TTS voice .dat into voice_data partition (offset 0x400000).
# ESP32-S3 / RAK3112: use -Voice xiaole (default).
# Usage: .\flash_voice.ps1 -Port COM5 [-Voice xiaole]
param(
    [Parameter(Mandatory = $true)][string]$Port,
    [ValidateSet("xiaole", "xiaoxin_small", "xiaoxin", "xiaoxin_custom")]
    [string]$Voice = "xiaole"
)

$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot
$DatName = "esp_tts_voice_data_$Voice.dat"
$DatOut = Join-Path $Root "data\$DatName"
$Offset = "0x400000"
$PartitionBytes = 0x300000

$VoiceInfo = @{
    xiaole         = "ESP32-S3 recommended: xiaole female voice (~2.8 MB)"
    xiaoxin_small  = "xiaoxin compact, robotic (~2.8 MB); needs TTS_VOICE_XIAOXIN in build_opt.h"
    xiaoxin        = "xiaoxin standard (~3.8 MB); partition too small by default"
    xiaoxin_custom = "xiaoxin custom (~3.7 MB); partition too small by default"
}

$EspSr = Join-Path $Root "esp-sr"
$DatSrc = Join-Path $EspSr "esp-tts\esp_tts_chinese\$DatName"
$DatUrl = "https://github.com/espressif/esp-sr/raw/master/esp-tts/esp_tts_chinese/$DatName"

function Ensure-VoiceFile {
    if (Test-Path $DatSrc) { return }
    if (-not (Test-Path $EspSr)) {
        Write-Host "Cloning esp-sr (voice data)..."
        git clone --depth 1 --filter=blob:none --sparse https://github.com/espressif/esp-sr.git $EspSr
        Set-Location $EspSr
        git sparse-checkout set esp-tts/esp_tts_chinese
        Set-Location $Root
    }
    if (-not (Test-Path $DatSrc)) {
        Write-Host "Downloading $DatUrl ..."
        New-Item -ItemType Directory -Force -Path (Split-Path $DatOut) | Out-Null
        curl.exe -L --connect-timeout 60 -o $DatOut $DatUrl
        if (-not (Test-Path $DatOut) -or (Get-Item $DatOut).Length -lt 100000) {
            Write-Host "ERROR: download failed: $DatOut"
            exit 1
        }
        return
    }
}

Write-Host "Voice: $Voice - $($VoiceInfo[$Voice])"
if ($Voice -ne "xiaole") {
    Write-Host "NOTE: ESP32-S3 Arduino 3.x links voice_set_xiaole only."
    Write-Host "      For xiaoxin voices add -DTTS_VOICE_XIAOXIN to build_opt.h and rebuild."
}

Ensure-VoiceFile

New-Item -ItemType Directory -Force -Path (Split-Path $DatOut) | Out-Null
if (Test-Path $DatSrc) {
    Copy-Item -Force $DatSrc $DatOut
}

if (-not (Test-Path $DatOut)) {
    Write-Host "ERROR: voice file not found: $DatOut"
    exit 1
}

$size = (Get-Item $DatOut).Length
if ($size -gt $PartitionBytes) {
    Write-Host "ERROR: $DatName is $size bytes but voice_data partition is $PartitionBytes bytes."
    Write-Host "       Enlarge voice_data in partitions.csv (e.g. 0x3C0000) and reflash firmware first."
    exit 1
}

$esptoolExe = Get-ChildItem (Join-Path $env:LOCALAPPDATA "Arduino15\packages\esp32\tools\esptool_py") -Recurse -Filter "esptool.exe" -ErrorAction SilentlyContinue |
    Sort-Object { [version]$_.Directory.Name } -Descending | Select-Object -First 1

if (-not $esptoolExe) {
    Write-Host "ERROR: esptool.exe not found. Install ESP32 board package in Arduino IDE."
    exit 1
}

Write-Host "Using $($esptoolExe.FullName)"
Write-Host "Flashing $DatOut ($($size) bytes) -> $Offset on $Port"
& $esptoolExe.FullName --chip esp32s3 --port $Port --baud 921600 write_flash $Offset $DatOut
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: esptool failed (exit $LASTEXITCODE). Close Serial Monitor if COM port is busy."
    exit $LASTEXITCODE
}
Write-Host "Done. Reboot board, then send STATUS and TTS command on serial."
