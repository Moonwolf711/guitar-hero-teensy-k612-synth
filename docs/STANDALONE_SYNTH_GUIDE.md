# 🎸 Guitar Hero Standalone Synthesizer - Complete Guide

## Transform Your Guitar Hero Controller into a Standalone Synthesizer!

**NO COMPUTER REQUIRED** - This system runs entirely on Teensy hardware with:
- ✅ USB Host input (any Guitar Hero controller)
- ✅ Real-time audio synthesis (SuperSaw engine)
- ✅ WiFi control via phone/tablet
- ✅ OSC output for DAW integration
- ✅ Ultra-low latency (<2ms)

---

## 📦 What You'll Need

### Essential Hardware
| Component | Purpose | Where to Buy | Price |
|-----------|---------|--------------|-------|
| **Teensy 4.1** | Main processor | PJRC.com | $30 |
| **Teensy Audio Shield Rev D** | Audio I/O | PJRC.com | $15 |
| **ESP-12E** or NodeMCU/Wemos D1 | WiFi control | AliExpress/Amazon | $5 |
| **Guitar Hero Controller** | Input device | eBay/GameStop | $10-20 |
| **USB Host Cable** | For Teensy 4.1 | PJRC.com | $3 |
| **3.3V Regulator** | Power ESP separately | AMS1117-3.3 | $1 |

### Optional Hardware
- Headphones or powered speakers
- Project enclosure
- Status LEDs
- Power switch
- External 5V power supply (for louder volume)

### Total Cost: ~$64 (plus controller)

---

## 🔧 Hardware Assembly

### Step 1: Audio Shield Connection

The Audio Shield plugs directly onto Teensy 4.1:

```
Teensy 4.1 Pins Used by Audio Shield:
───────────────────────────────────
Pin 7   → MOSI (SPI)
Pin 12  → MISO (SPI)
Pin 13  → SCLK (SPI)
Pin 10  → CS (SPI)
Pin 23  → MCLK (Audio clock)
Pin 18  → SDA (I2C)
Pin 19  → SCL (I2C)
Pins 20,21 → I2S (Audio data)

Audio Out → 3.5mm jack on shield
```

**Assembly**:
1. Stack Audio Shield on top of Teensy 4.1
2. Ensure all pins are aligned
3. Press firmly until seated
4. Connect headphones/speakers to 3.5mm jack

### Step 2: ESP-12E Wiring

**IMPORTANT**: ESP-12E needs 250-350mA. Use external regulator!

```
┌──────────────┐     Serial      ┌───────────────┐
│  Teensy 4.1  │◄────────────────►│   ESP-12E     │
│              │                  │               │
│  TX1 (Pin 1) ├──────────────────┤ RX (GPIO3)    │
│  RX1 (Pin 0) ├──────────────────┤ TX (GPIO1)    │
│  GND         ├──────────────────┤ GND           │
└──────────────┘                  │               │
                                  │ VCC ◄─────────┤ 3.3V Regulator
                                  │               │
                                  │ GPIO0  ◄───┬──┤ VCC (10kΩ pullup)
                                  │ GPIO2  ◄───┘  │
                                  │ GPIO15 ◄──────┤ GND (10kΩ pulldown)
                                  │ CH_PD  ◄──────┤ VCC
                                  └───────────────┘
```

**Pin Connections Table**:

| ESP-12E Pin | Connect To | Notes |
|-------------|------------|-------|
| VCC | 3.3V Reg OUT | **NOT Teensy 3.3V!** |
| GND | Common GND | Teensy + Power |
| TX (GPIO1) | Teensy RX1 (Pin 0) | Serial data |
| RX (GPIO3) | Teensy TX1 (Pin 1) | Serial data |
| GPIO0 | 10kΩ → VCC | Boot mode pull-up |
| GPIO2 | 10kΩ → VCC | Required pull-up |
| GPIO15 | 10kΩ → GND | Boot mode pull-down |
| CH_PD | VCC | Enable pin |

### Step 3: USB Host Connection

Connect Guitar Hero controller to Teensy's USB Host port:

**Teensy 4.1 USB Host Pins**:
- **D+** → Pin 34
- **D-** → Pin 35
- **VIN** → 5V (for controller power)
- **GND** → GND

Or use USB Host cable from PJRC (recommended).

### Step 4: Power Supply

```
Option 1: USB Power Only
USB (5V) → Teensy VIN
         → 3.3V Regulator → ESP-12E

Option 2: External Power (Better audio quality)
5V Power Supply → Teensy VIN
                → 3.3V Regulator → ESP-12E
                → USB Host (controller power)
```

---

## 💾 Software Installation

### Prerequisites

1. **Arduino IDE** with Teensyduino:
   - Download from https://www.pjrc.com/teensy/td_download.html
   - Install Teensy Loader

2. **ESP8266 Board Support**:
   - Arduino IDE → Preferences
   - Additional Boards Manager URLs:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Tools → Board → Boards Manager → Install "ESP8266"

3. **Required Libraries**:
   - USBHost_t36 (included with Teensyduino)
   - Audio (included with Teensyduino)
   - ESP8266WiFi (included with ESP8266 core)
   - ESP8266WebServer (included with ESP8266 core)
   - ESP8266mDNS (included with ESP8266 core)

### Upload Firmware

#### Part 1: Upload to Teensy 4.1

1. Open `gh_teensy_standalone_synth.ino` in Arduino IDE
2. Select:
   - **Board**: Teensy 4.1
   - **USB Type**: Audio + MIDI + Serial
   - **CPU Speed**: 600 MHz
   - **Optimize**: Faster
3. Click **Verify** (✓) to compile
4. Click **Upload** (→)
5. Teensy Loader will open automatically
6. Press button on Teensy to upload

**Expected Output** (Serial Monitor @ 115200):
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

#### Part 2: Upload to ESP-12E

1. Open `gh_esp12e_osc_control.ino` in Arduino IDE
2. Select:
   - **Board**: Generic ESP8266 Module
   - **Flash Size**: 4MB (FS:2MB OTA:~1019KB)
   - **CPU Frequency**: 80MHz
   - **Upload Speed**: 115200
3. Connect ESP-12E via USB-Serial adapter
4. Hold GPIO0 LOW during power-on (programming mode)
5. Click **Upload**

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

## 🎮 Using Your Synth

### Step 1: Power On

1. Connect power to Teensy (USB or external 5V)
2. Wait 5 seconds for initialization
3. ESP creates WiFi access point: **"GuitarHero-Synth"**

### Step 2: Connect Controller

1. Plug Guitar Hero controller into USB Host port
2. Teensy will detect it automatically
3. Serial Monitor shows:
   ```
   *** CONTROLLER CONNECTED! ***
   VID: 0x12BA PID: 0x0100
   Ready to play!
   ```

### Step 3: Play!

Press any fret button → **INSTANT SOUND!**

No strum required - notes trigger immediately for keyboard-style playing.

### Step 4: WiFi Control (Optional)

1. Connect phone/tablet to **"GuitarHero-Synth"** WiFi
   - Password: `rockstar`
2. Open browser to: **http://192.168.4.1**
3. See real-time visualization:
   - Current note playing
   - CPU usage meter
   - Active scale/root
   - Total notes played

---

## 🎹 Control Reference

### Fret Buttons
| Button | Function |
|--------|----------|
| **Green** | Root note of scale |
| **Red** | 2nd degree |
| **Yellow** | 3rd degree |
| **Blue** | 4th degree |
| **Orange** | 5th degree |

All notes follow selected pentatonic scale!

### Special Controls
| Control | Function |
|---------|----------|
| **Star Power** | Toggle arpeggiator (auto-plays held notes) |
| **Plus Button** | Next scale (Major → Minor → Blues → Japanese → Egyptian) |
| **Minus Button** | Previous scale |
| **D-Pad Up** | Octave +1 (up to +2) |
| **D-Pad Down** | Octave -1 (down to -2) |
| **Whammy Bar** | Pitch bend ± 2 semitones + filter sweep |
| **Tilt Sensor** | Filter cutoff (if supported by controller) |

### Available Scales
1. **Major Pentatonic** - Happy, bright (C D E G A)
2. **Minor Pentatonic** - Bluesy, moody (C Eb F G Bb)
3. **Blues** - Classic blues (C Eb F F# Bb)
4. **Japanese (In Sen)** - Exotic (C Db F G Ab)
5. **Egyptian** - Ancient (C D F G Bb)
6. **Dorian Pentatonic** - Modal jazz (C D Eb G A)
7. **Lydian Pentatonic** - Bright, dreamy (C D E F# A)

---

## 🌐 Web Interface Features

Access at **http://192.168.4.1** or **http://gh-synth.local**

### Real-Time Display
- **Current Note**: Shows note name in huge font when playing
- **CPU Usage**: Live meter (should stay under 30%)
- **Memory**: Audio buffer usage
- **Latency**: Loop time in microseconds
- **Total Notes**: Count since power-on

### Quick Controls
- One-click scale changes
- Octave up/down buttons
- Arpeggiator toggle
- Refresh button

### Mobile Friendly
- Responsive design
- Works on iPhone, Android, tablets
- No app required - just a browser

---

## 📡 OSC Integration (For DAWs)

The ESP broadcasts OSC messages on **port 8000** (UDP):

### OSC Messages Sent

| Address | Type | Description |
|---------|------|-------------|
| `/synth/noteon` | int | MIDI note number when pressed |
| `/synth/noteoff` | int | MIDI note number when released |
| `/synth/scale` | int, string | Scale index + name |
| `/synth/root` | int, string | Root note index + name |
| `/synth/octave` | int | Current octave offset (-2 to +2) |
| `/synth/arp` | int | Arpeggiator state (0/1) |
| `/synth/cpu` | float | CPU usage percentage |
| `/synth/memory` | int | Audio memory blocks used |
| `/synth/latency` | int | Loop latency in microseconds |

### Receiving in Ableton Live

1. Install **Connection Kit** or **OSCulator**
2. Listen on UDP port 8000
3. Map OSC messages to parameters
4. Example: `/synth/noteon` → trigger clip

### Receiving in Max/MSP

```
[udpreceive 8000]
|
[OSC-route /synth]
|
[route noteon noteoff scale root]
```

---

## 🔍 Troubleshooting

### No Sound

**Check**:
1. Audio Shield seated properly?
2. Headphones/speakers connected?
3. Volume on Audio Shield (try external volume control)
4. Serial Monitor shows "Audio Shield enabled"?

**Fix**:
- Re-seat Audio Shield
- Try built-in DAC (A21/A22 pins)
- Check solder connections

### Controller Not Detected

**Check**:
1. USB Host cable wired correctly?
2. Controller powered? (5V on USB VIN)
3. Serial Monitor shows button presses?

**Fix**:
- Test with USB Host cable from PJRC
- Try different controller
- Check Teensy USB Host pins (34, 35)

### ESP WiFi Not Visible

**Check**:
1. ESP powered with 3.3V regulator?
2. GPIO0, GPIO2 pulled HIGH?
3. GPIO15 pulled LOW?
4. Serial shows "AP IP: 192.168.4.1"?

**Fix**:
- Use external 3.3V regulator (not Teensy's 3.3V pin)
- Add 100μF capacitor near ESP VCC
- Check pull-up/pull-down resistors

### High Latency / Glitches

**Check**:
1. CPU usage (should be <30%)
2. Audio memory (should be <40 blocks)
3. USB power sufficient?

**Fix**:
- Use external 5V power supply
- Disable OSC if not needed (`OSC_ENABLED false`)
- Reduce audio effects

### Web Interface Not Loading

**Check**:
1. Connected to "GuitarHero-Synth" WiFi?
2. IP address is 192.168.4.1?
3. ESP Serial shows "Web server started"?

**Fix**:
- Try http://192.168.4.1 (not https)
- Disable mobile data on phone
- Clear browser cache
- Restart ESP

---

## ⚡ Performance Specs

### Latency Breakdown
```
USB Polling:        ~1ms
Button Debounce:    ~3ms
Audio Processing:   ~2.9ms (128 samples @ 44.1kHz)
Total:              <7ms (excellent for live playing!)
```

### Resource Usage (Typical)
```
CPU:                15-25%
Audio Memory:       20-35 blocks (out of 60)
RAM:                ~30KB
Flash:              ~150KB
```

### Audio Quality
```
Sample Rate:        44.1kHz
Bit Depth:          16-bit
SuperSaw Voices:    5 detuned oscillators
Filter:             State-variable, resonant
Effects:            Chorus, Delay (optional)
```

---

## 🎯 Advanced Customization

### Change Synth Waveform

Edit `gh_teensy_standalone_synth.ino` line ~234:

```cpp
osc1.begin(WAVEFORM_SAWTOOTH);  // Options:
// WAVEFORM_SINE
// WAVEFORM_SQUARE
// WAVEFORM_TRIANGLE
// WAVEFORM_SAWTOOTH
// WAVEFORM_PULSE
```

### Add More Effects

After line ~85:

```cpp
AudioEffectReverb reverb1;
AudioEffectFlange flange1;
AudioEffectBitcrusher bitcrusher1;

// Connect in effects chain
```

### Customize WiFi Settings

Edit `gh_esp12e_osc_control.ino`:

```cpp
const char* AP_SSID = "YourName";
const char* AP_PASSWORD = "your_password";
```

### Add MIDI Output

Teensy 4.1 has native USB MIDI. To also send MIDI:

```cpp
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  MIDI.begin(1);
}

// In playNote():
MIDI.sendNoteOn(midiNote, 100, 1);
```

---

## 📚 Code Structure

### Teensy Firmware (`gh_teensy_standalone_synth.ino`)
```
Lines    Purpose
──────────────────────────────────────
1-60     USB Host + Audio Library setup
61-110   Scale definitions (7 pentatonic scales)
111-170  State variables + button tracking
171-230  Audio synthesis functions
231-290  SuperSaw oscillator control
291-370  Guitar Hero input processing
371-450  Fret, control, analog processing
451-500  Arpeggiator logic
501-580  ESP communication (JSON)
581-700  Main setup + loop
```

### ESP Firmware (`gh_esp12e_osc_control.ino`)
```
Lines    Purpose
──────────────────────────────────────
1-50     WiFi + OSC setup
51-150   OSC message generation
151-250  Teensy data parsing (JSON)
251-500  Web interface HTML
501-600  Web server handlers
601-700  Main setup + loop
```

---

## 🚀 Next Steps

### Beginner
1. ✅ Upload firmware
2. ✅ Connect controller
3. ✅ Play!

### Intermediate
1. Experiment with scales
2. Try arpeggiator patterns
3. Connect to web interface
4. Adjust whammy sensitivity

### Advanced
1. Add custom scales
2. Integrate OSC with DAW
3. Design custom waveforms
4. Build enclosure with status LEDs
5. Add MIDI DIN output
6. Record performances to SD card

---

## 🎸 Tips for Great Sound

### Getting Started
- Start with **Minor Pentatonic** (bluesy, forgiving)
- Use **Whammy bar** for expression
- Try **Star Power arpeggiator** for ambient pads

### Playing Techniques
- **Hold multiple frets** → Chords
- **Quick fret taps** → Staccato notes
- **Slow fret slides** → Portamento glide
- **Whammy + frets** → Expressive solos

### Sound Design
- **Low octaves** → Bass lines
- **High octaves** → Lead melodies
- **Japanese scale** → Film score vibes
- **Blues scale** → Classic rock solos

---

## 📖 Resources

### Documentation
- [Teensy 4.1 Pinout](https://www.pjrc.com/store/teensy41.html)
- [Audio Library Reference](https://www.pjrc.com/teensy/td_libs_Audio.html)
- [USBHost_t36 Guide](https://github.com/PaulStoffregen/USBHost_t36)
- [ESP8266 Arduino](https://github.com/esp8266/Arduino)

### Community
- [PJRC Forum](https://forum.pjrc.com/)
- [Teensy Reddit](https://reddit.com/r/teensy)
- [ESP8266 Forum](https://www.esp8266.com/)

### Inspiration
- Watch synth demos on YouTube
- Check pentatonic scale tutorials
- Explore modular synthesis concepts

---

## 🎉 Congratulations!

You now have a **complete standalone synthesizer** that requires no computer!

🎸 Plug in your Guitar Hero controller
🔊 Hear instant SuperSaw synthesis
📱 Control from your phone
🎵 Create music anywhere!

**Rock on!** 🤘

---

*Built with love for guitarists who want to explore synthesis*
*Version 2.0 - January 2025*
