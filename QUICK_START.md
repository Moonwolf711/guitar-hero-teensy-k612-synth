# 🚀 Quick Start - Upload Firmware

Your complete Guitar Hero Standalone Synthesizer firmware is ready!

## 📂 Project Structure

```
/home/moon_wolf/guitar-hero-teensy-k612-synth/
├── firmware/
│   ├── gh_teensy_standalone_synth.ino    ← Upload to Teensy 4.1
│   └── gh_esp12e_osc_control.ino         ← Upload to ESP-12E
├── docs/
│   ├── FIRMWARE_UPLOAD_GUIDE.md          ← Detailed upload instructions
│   └── STANDALONE_SYNTH_GUIDE.md         ← Complete hardware/software guide
└── QUICK_START.md                        ← This file
```

---

## ⚡ Quick Upload Steps

### 1️⃣ Upload Teensy Firmware

**From Windows (Arduino IDE)**:
1. Open Arduino IDE
2. File → Open → `C:\Users\Owner\gh_midi_controller_optimized\gh_teensy_standalone_synth.ino`
   - Or access via Windows: `/mnt/c/Users/Owner/gh_midi_controller_optimized/`
   - Or from Linux path: `/home/moon_wolf/guitar-hero-teensy-k612-synth/firmware/`
3. Tools → Board → **Teensy 4.1**
4. Tools → USB Type → **Audio + MIDI + Serial** ⚠️ CRITICAL!
5. Tools → CPU Speed → **600 MHz**
6. Click Upload (→)
7. Press button on Teensy when prompted

**Expected Output** (Serial Monitor @ 115200):
```
╔════════════════════════════════════════════════════╗
║  GUITAR HERO STANDALONE SYNTHESIZER v2.0          ║
║  Teensy 4.1 + Audio Shield + ESP-12E WiFi         ║
╚════════════════════════════════════════════════════╝

✓ USB Host initialized
✓ Audio synthesis initialized
🎸 READY TO ROCK!
```

### 2️⃣ Upload ESP Firmware (Optional)

**From Windows (Arduino IDE)**:
1. File → Open → `gh_esp12e_osc_control.ino`
2. Tools → Board → **Generic ESP8266 Module** (or NodeMCU)
3. Tools → Flash Size → **4MB (FS:2MB OTA:~1019KB)**
4. Connect GPIO0 to GND (programming mode)
5. Click Upload
6. Wait ~60 seconds

**Expected Output** (Serial Monitor @ 115200):
```
=== ESP-12E OSC Control ===
AP IP: 192.168.4.1
mDNS: http://gh-synth.local
OSC: Port 8000
Web server started
Ready!
```

---

## 🔗 Access From Windows or Linux

### Windows Path:
```
C:\Users\Owner\gh_midi_controller_optimized\
```

### Linux/WSL Path:
```
/home/moon_wolf/guitar-hero-teensy-k612-synth/firmware/
```

### Or via WSL mount:
```
/mnt/c/Users/Owner/gh_midi_controller_optimized/
```

All paths work - use whichever is convenient!

---

## 📚 Documentation

- **Upload Instructions**: `docs/FIRMWARE_UPLOAD_GUIDE.md`
- **Hardware Guide**: `docs/STANDALONE_SYNTH_GUIDE.md`
- **Session Summary**: `docs/session-summary-2025-10-30.md`

---

## ✅ What You Get

### Without ESP-12E (Basic)
- ✅ Standalone synthesizer
- ✅ USB Host Guitar Hero input
- ✅ SuperSaw audio synthesis
- ✅ 7 pentatonic scales
- ✅ Arpeggiator
- ✅ Whammy effects

### With ESP-12E (Full System)
- ✅ Everything above PLUS:
- ✅ WiFi web interface
- ✅ Real-time note visualization
- ✅ OSC output (DAW integration)
- ✅ Remote control from phone

---

## 🎸 Features

- **7 Scales**: Major Pent, Minor Pent, Blues, Japanese, Egyptian, Dorian, Lydian
- **Strum Transpose**: ±12 semitones (octave shifts)
- **Arpeggiator**: Star Power button activates
- **Whammy Bar**: Pitch bend + filter sweep
- **D-Pad**: Octave control
- **WiFi Control**: Web UI at http://192.168.4.1

---

## 🆘 Need Help?

1. **Upload Issues**: See `docs/FIRMWARE_UPLOAD_GUIDE.md`
2. **Hardware Wiring**: See `docs/STANDALONE_SYNTH_GUIDE.md`
3. **Compilation Errors**: Check you have Teensyduino installed

---

**Ready to upload!** 🚀

Open Arduino IDE and start with the Teensy firmware!
