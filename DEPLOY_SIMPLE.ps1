# Guitar Hero Teensy Synthesizer - Simple Deployment Script
# This script automates uploading firmware to your Teensy 4.1

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Guitar Hero Teensy Synth Deployer" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Find Teensy Loader
Write-Host "[1/4] Looking for Teensy Loader..." -ForegroundColor Yellow

$teensyPaths = @(
    "${env:USERPROFILE}\.platformio\packages\tool-teensy\teensy_loader_cli.exe",
    "${env:ProgramFiles(x86)}\Arduino\hardware\tools\teensy_loader_cli.exe",
    "${env:ProgramFiles}\Arduino\hardware\tools\teensy_loader_cli.exe",
    "${env:LOCALAPPDATA}\Arduino15\packages\teensy\tools\teensy-tools\1.59.0\teensy_loader_cli.exe",
    "C:\Program Files (x86)\Arduino\hardware\teensy\avr\tools\teensy_post_compile.exe"
)

$teensyLoader = $null
foreach ($path in $teensyPaths) {
    if (Test-Path $path) {
        $teensyLoader = $path
        Write-Host "[OK] Found Teensy tool at: $path" -ForegroundColor Green
        break
    }
}

# Step 2: Look for compiled HEX file
Write-Host ""
Write-Host "[2/4] Looking for compiled firmware..." -ForegroundColor Yellow

$hexFiles = @(
    "gh_teensy_standalone_synth.ino.hex",
    "build\gh_teensy_standalone_synth.ino.hex",
    "firmware\gh_teensy_standalone_synth.ino.hex"
)

$hexFile = $null
foreach ($file in $hexFiles) {
    if (Test-Path $file) {
        $hexFile = $file
        Write-Host "[OK] Found HEX file: $file" -ForegroundColor Green
        break
    }
}

if (-not $hexFile) {
    Write-Host "[FAIL] No compiled HEX file found" -ForegroundColor Red
    Write-Host ""
    Write-Host "You need to compile first using Arduino IDE:" -ForegroundColor Yellow
    Write-Host "=================================================" -ForegroundColor Cyan
    Write-Host "1. Open Arduino IDE" -ForegroundColor White
    Write-Host "2. File -> Open -> gh_teensy_standalone_synth.ino" -ForegroundColor White
    Write-Host "3. Tools -> Board -> Teensy 4.1" -ForegroundColor White
    Write-Host "4. Tools -> USB Type -> Audio + MIDI + Serial (CRITICAL!)" -ForegroundColor White
    Write-Host "5. Tools -> CPU Speed -> 600 MHz" -ForegroundColor White
    Write-Host "6. Sketch -> Verify/Compile (checkmark button)" -ForegroundColor White
    Write-Host "7. Wait for 'Done compiling' message" -ForegroundColor White
    Write-Host "8. Then run this script again OR click Upload" -ForegroundColor White
    Write-Host "=================================================" -ForegroundColor Cyan
    Write-Host ""

    # Open Arduino IDE automatically
    Write-Host "Would you like to open Arduino IDE now? (Y/N): " -ForegroundColor Yellow -NoNewline
    $response = Read-Host

    if ($response -eq "Y" -or $response -eq "y") {
        $arduinoExe = "${env:ProgramFiles(x86)}\Arduino\arduino.exe"
        if (Test-Path $arduinoExe) {
            Write-Host "Opening Arduino IDE..." -ForegroundColor Cyan
            Start-Process $arduinoExe -ArgumentList "gh_teensy_standalone_synth.ino"
        } else {
            Write-Host "Arduino IDE not found at default location" -ForegroundColor Red
        }
    }

    exit 1
}

# Step 3: Check for Teensy device
Write-Host ""
Write-Host "[3/4] Checking for Teensy device..." -ForegroundColor Yellow

$comPorts = Get-WmiObject Win32_SerialPort | Where-Object { $_.Description -like "*Teensy*" -or $_.Description -like "*USB Serial*" }

if ($comPorts) {
    Write-Host "[OK] Found COM ports:" -ForegroundColor Green
    $comPorts | ForEach-Object {
        Write-Host "  * $($_.DeviceID) - $($_.Description)" -ForegroundColor White
    }
} else {
    Write-Host "[WARNING] No Teensy detected on COM ports" -ForegroundColor Yellow
    Write-Host "  Make sure Teensy is connected via USB" -ForegroundColor White
}

# Step 4: Upload
Write-Host ""
Write-Host "[4/4] Ready to upload firmware!" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Firmware: gh_teensy_standalone_synth.ino" -ForegroundColor White
Write-Host "Target: Teensy 4.1" -ForegroundColor White
Write-Host "USB Type: Audio + MIDI + Serial" -ForegroundColor White
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "IMPORTANT: Press the WHITE BUTTON on your Teensy when prompted!" -ForegroundColor Cyan
Write-Host ""
Write-Host "Press ENTER to start upload..." -ForegroundColor Yellow
Read-Host

if ($teensyLoader -and (Test-Path $teensyLoader)) {
    Write-Host "Uploading via Teensy Loader..." -ForegroundColor Cyan
    & $teensyLoader --mcu=TEENSY41 -w -v $hexFile

    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "[SUCCESS] UPLOAD COMPLETE!" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "[FAIL] Upload failed - try manual upload" -ForegroundColor Red
    }
} else {
    Write-Host "Automated upload not available." -ForegroundColor Yellow
    Write-Host "Please use Arduino IDE:" -ForegroundColor Yellow
    Write-Host "1. File -> Open -> gh_teensy_standalone_synth.ino" -ForegroundColor White
    Write-Host "2. Click Upload button (arrow)" -ForegroundColor White
    Write-Host "3. Press button on Teensy when prompted" -ForegroundColor White
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "* Open Serial Monitor at 115200 baud" -ForegroundColor White
Write-Host "* Connect your Guitar Hero controller" -ForegroundColor White
Write-Host "* Test fret buttons and strum bar" -ForegroundColor White
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
