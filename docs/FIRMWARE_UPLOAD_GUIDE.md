# 🚀 Firmware Upload Guide - Step by Step

This guide will walk you through uploading firmware to **both** devices:
1. Teensy 4.1 (main synthesizer)
2. ESP-12E (WiFi control)

---

## ✅ Pre-Flight Checklist

Before we start, make sure you have:

- [ ] **Arduino IDE** installed
- [ ] **Teensyduino** add-on installed
- [ ] **ESP8266 board support** installed
- [ ] **USB cable** for Teensy
- [ ] **USB-to-Serial adapter** for ESP (or NodeMCU/Wemos)
- [ ] Files ready in `C:\Users\Owner\gh_midi_controller_optimized\`

---

## 📥 Part 1: Install Software (If Not Already Done)

### Step 1.1: Install Arduino IDE

1. Download from: https://www.arduino.cc/en/software
2. Choose **Windows installer** (not Windows app)
3. Install to default location: `C:\Program Files (x86)\Arduino`
4. Launch Arduino IDE

### Step 1.2: Install Teensyduino

1. Download from: https://www.pjrc.com/teensy/td_download.html
2. **Close Arduino IDE** before installing!
3. Run Teensyduino installer
4. Select Arduino installation folder
5. Install **all libraries** (especially Audio and USBHost_t36)
6. Click Install

**Verification**:
- Arduino IDE → Tools → Board → Should see "Teensy 4.1"
- Arduino IDE → File → Examples → Should see "Audio" and "USBHost_t36"

### Step 1.3: Install ESP8266 Support

1. Open Arduino IDE
2. **File → Preferences**
3. Find "Additional Boards Manager URLs" field
4. Add this URL:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
5. Click OK
6. **Tools → Board → Boards Manager**
7. Search for "ESP8266"
8. Install **"esp8266 by ESP8266 Community"** (latest version)
9. Wait for download (~200MB)
10. Close Boards Manager

**Verification**:
- Tools → Board → Should see "ESP8266 Boards" section

---

## 🎸 Part 2: Upload to Teensy 4.1

### Step 2.1: Connect Teensy

1. Connect Teensy 4.1 to your computer via USB
2. Wait for Windows to recognize it
3. Don't worry if it says "USB Serial Device" - that's normal

### Step 2.2: Open Firmware

1. In Arduino IDE: **File → Open**
2. Navigate to:
   ```
   C:\Users\Owner\gh_midi_controller_optimized\
   ```
3. Open: **`gh_teensy_standalone_synth.ino`**

### Step 2.3: Configure Arduino IDE

Set these settings in **Tools** menu:

| Setting | Value |
|---------|-------|
| **Board** | Teensy 4.1 |
| **USB Type** | **Audio + MIDI + Serial** ← IMPORTANT! |
| **CPU Speed** | 600 MHz |
| **Optimize** | Faster |
| **Port** | (Auto-detect) |
| **Keyboard Layout** | US English |

**Screenshot reference**:
```
Tools
  ├─ Board: "Teensy 4.1" ✓
  ├─ USB Type: "Audio + MIDI + Serial" ✓
  ├─ CPU Speed: "600 MHz" ✓
  ├─ Optimize: "Faster" ✓
  └─ Port: "COMx (Teensy)" ✓
```

### Step 2.4: Compile and Upload

1. Click **Verify** button (✓) first to check for errors
   - Wait for compilation (30-60 seconds)
   - Should say "Done compiling" at bottom

2. If successful, click **Upload** button (→)
   - **Teensy Loader** will open automatically
   - Wait for upload (10-20 seconds)

3. **Press the white button on Teensy** when Teensy Loader says:
   ```
   Press button to upload...
   ```

4. Upload complete! Should see:
   ```
   Reboot OK
   ```

### Step 2.5: Test Teensy

1. **Tools → Serial Monitor** (or Ctrl+Shift+M)
2. Set baud rate to **115200**
3. You should see:
   ```
   ╔════════════════════════════════════════════════════╗
   ║  GUITAR HERO STANDALONE SYNTHESIZER v2.0          ║
   ║  Teensy 4.1 + Audio Shield + ESP-12E WiFi         ║
   ╚════════════════════════════════════════════════════╝

   ✓ USB Host initialized
   ✓ Audio Shield enabled
   ✓ Audio synthesis initialized
   ✓ ESP-12E communication initialized

   🎸 READY TO ROCK!
   ```

**Success!** ✅ Teensy firmware is uploaded!

---

## 📡 Part 3: Upload to ESP-12E

### Step 3.1: Choose Your ESP Module

**Do you have**:
- **A) Bare ESP-12E module** → Needs USB-to-Serial adapter
- **B) NodeMCU or Wemos D1 Mini** → Has USB built-in ✅ (easier!)

### Option A: Bare ESP-12E Module

#### Wiring for Programming

```
USB-to-Serial Adapter     ESP-12E Module
─────────────────────     ──────────────
3.3V      ──────────────► VCC
GND       ──────────────► GND
TX        ──────────────► RX (GPIO3)
RX        ──────────────► TX (GPIO1)

                          GPIO0   ──► GND (via button)
                          GPIO2   ──► VCC (10kΩ)
                          GPIO15  ──► GND (10kΩ)
                          CH_PD   ──► VCC
                          RST     ──► VCC (add button to GND)
```

**Important**: Cross TX/RX (adapter TX → ESP RX, adapter RX → ESP TX)

### Option B: NodeMCU or Wemos D1 Mini

Just plug in via USB cable! Much easier. 😊

### Step 3.2: Enter Programming Mode

**For bare ESP-12E**:
1. Connect GPIO0 to GND (hold button)
2. Power on (or press RST button)
3. Release GPIO0 button
4. ESP is now in programming mode

**For NodeMCU/Wemos**:
- Just plug it in! (GPIO0 button may be on board)

### Step 3.3: Open ESP Firmware

1. In Arduino IDE: **File → Open**
2. Navigate to:
   ```
   C:\Users\Owner\gh_midi_controller_optimized\
   ```
3. Open: **`gh_esp12e_osc_control.ino`**

### Step 3.4: Configure Arduino IDE

Set these settings in **Tools** menu:

| Setting | Value |
|---------|-------|
| **Board** | Generic ESP8266 Module |
| **Upload Speed** | 115200 |
| **CPU Frequency** | 80 MHz |
| **Flash Size** | 4MB (FS:2MB OTA:~1019KB) |
| **Flash Mode** | DIO |
| **Port** | (Your USB-Serial COM port) |

**For NodeMCU/Wemos**, select:
- Board: "NodeMCU 1.0 (ESP-12E Module)" or "LOLIN(WEMOS) D1 R2 & mini"

### Step 3.5: Find COM Port

**Windows**:
1. Open Device Manager
2. Look under "Ports (COM & LPT)"
3. Should see "USB-SERIAL CH340" or similar with COM number
4. Note the **COM port** (e.g., COM3, COM5)

**Arduino IDE**:
1. Tools → Port
2. Select the COM port you found
3. Should show board name in parentheses

### Step 3.6: Compile and Upload

1. Click **Verify** button (✓)
   - Wait for compilation (20-40 seconds)
   - Should say "Done compiling"

2. **Make sure ESP is in programming mode!**
   - For bare ESP-12E: GPIO0 connected to GND
   - For NodeMCU: Hold "FLASH" button if needed

3. Click **Upload** button (→)
   - You'll see dots in Serial Monitor:
     ```
     .....................
     ```
   - This is normal! Upload takes 30-60 seconds

4. When complete, should see:
   ```
   Leaving...
   Hard resetting via RTS pin...
   ```

### Step 3.7: Test ESP

1. **Remove GPIO0 from GND** (if bare module)
2. Press RST button or reconnect power
3. **Tools → Serial Monitor**
4. Set baud rate to **115200**
5. Should see:
   ```
   === ESP-12E OSC Control ===
   AP IP: 192.168.4.1
   mDNS: http://gh-synth.local
   OSC: Port 8000
   Web server started
   Ready!
   ```

**Success!** ✅ ESP firmware is uploaded!

---

## 🔗 Part 4: Connect Hardware Together

Now that both firmwares are uploaded, connect Teensy and ESP:

### Final Wiring

```
Teensy 4.1        ESP-12E (or NodeMCU)
──────────        ────────────────────
Pin 1 (TX1)   →   RX (GPIO3)
Pin 0 (RX1)   →   TX (GPIO1)
GND           →   GND

Power ESP with 3.3V regulator!
```

**For NodeMCU/Wemos**: Use the labeled TX/RX pins.

---

## ✅ Part 5: Final Test

### Test 1: Serial Communication

1. Connect both Teensy and ESP via wiring
2. Power on Teensy (via USB)
3. Power on ESP (via regulator or separate USB)
4. Open **two Serial Monitors** (use Arduino Serial Monitor + another tool like PuTTY)
5. Watch for messages passing between them

### Test 2: WiFi Access Point

1. On your phone/computer WiFi settings
2. Look for network: **"GuitarHero-Synth"**
3. Connect with password: **`rockstar`**
4. Open browser to: **http://192.168.4.1**
5. Should see web interface!

### Test 3: Controller Input

1. Connect Guitar Hero controller to Teensy USB Host port
2. Serial Monitor should show:
   ```
   *** CONTROLLER CONNECTED! ***
   ```
3. Press fret buttons
4. Serial Monitor shows button presses
5. **Hear audio** if Audio Shield is connected!

---

## 🐛 Troubleshooting Upload Issues

### Teensy Won't Upload

**Error**: "Teensy did not respond"

**Solutions**:
1. Press white button on Teensy
2. Try different USB cable
3. Reinstall Teensy drivers
4. Close all Serial Monitors
5. Disconnect and reconnect USB

### ESP Won't Upload

**Error**: "Failed to connect to ESP"

**Solutions**:
1. Check GPIO0 is connected to GND during upload
2. Try lower upload speed (9600 or 57600)
3. Check TX/RX are **crossed** (adapter TX → ESP RX)
4. Verify 3.3V power (use multimeter)
5. Add 10µF capacitor between VCC and GND
6. Try different USB-Serial adapter (CH340, CP2102, FTDI)

**Error**: "Sketch too large"

**Solutions**:
1. Select correct Flash Size (4MB minimum)
2. Try different partition scheme
3. Remove debug code if you added any

### Compilation Errors

**Error**: "Audio.h: No such file"

**Solution**: Install Teensyduino (includes Audio library)

**Error**: "ESP8266WiFi.h: No such file"

**Solution**: Install ESP8266 board support in Boards Manager

**Error**: "USBHost_t36.h: No such file"

**Solution**: Update Teensyduino to latest version

---

## 📊 Expected Output

### Successful Teensy Upload
```
Sketch uses 152,432 bytes (7%) of program storage space
Global variables use 63,156 bytes (12%) of dynamic memory
Teensy Loader: Uploading...
Reboot OK
```

### Successful ESP Upload
```
Sketch uses 298,756 bytes (28%) of program storage space
Global variables use 28,512 bytes (34%) of dynamic memory
Leaving...
Hard resetting via RTS pin...
```

---

## 🎯 Quick Reference Commands

### Open Serial Monitor
- **Windows**: Ctrl+Shift+M
- **Mac**: Cmd+Shift+M

### Select Board
- **Tools → Board → [Your Board]**

### Check Port
- **Tools → Port → [Your COM Port]**

### Upload Firmware
- **Sketch → Upload** (or Ctrl+U)

### Verify Only
- **Sketch → Verify/Compile** (or Ctrl+R)

---

## 📝 Checklist: Did Everything Work?

After uploading, verify:

- [ ] Teensy Serial Monitor shows initialization messages
- [ ] ESP Serial Monitor shows "Web server started"
- [ ] WiFi network "GuitarHero-Synth" is visible
- [ ] Web interface loads at http://192.168.4.1
- [ ] Controller connects to Teensy (if plugged in)
- [ ] Audio output works (if Audio Shield connected)

If all checked ✅ → **You're done!** 🎉

---

## 🆘 Still Having Issues?

### Get More Help

1. **Check Serial Monitor output** - Copy exact error messages
2. **Take photos** of your wiring
3. **Note which step failed** - Upload? Compilation? Connection?

### Common Issues Table

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| Teensy not detected | Driver issue | Reinstall Teensyduino |
| ESP upload fails | Wrong mode | Connect GPIO0 to GND |
| Compilation errors | Missing library | Install via Library Manager |
| No WiFi signal | Power issue | Use 3.3V regulator |
| No audio | Shield not detected | Check solder connections |

---

## 🎉 Next Steps

Once firmware is uploaded successfully:

1. **Read** `STANDALONE_SYNTH_GUIDE.md` for wiring Audio Shield
2. **Connect** your Guitar Hero controller
3. **Test** by pressing fret buttons
4. **Explore** the web interface
5. **Rock out!** 🎸

---

*Good luck with your upload! You're one step away from a standalone synthesizer!* 🚀
