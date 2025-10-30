# Build Instructions for Guitar Hero Teensy Synthesizer

## Prerequisites

### Software Requirements
- **PlatformIO Core** (CLI) or **PlatformIO IDE** (VS Code extension)
- **Arduino IDE 1.8.x or 2.x** with Teensyduino (alternative to PlatformIO)
- **Python 3.7+** (for PlatformIO)
- **Git** (for cloning repository)
- **Node.js** (optional, for web interface development)

### Hardware Requirements
- **Teensy 4.1** microcontroller board
- **ESP8266-12E** module (NodeMCU or similar)
- **Xbox 360 Guitar Hero Controller** (wired USB or wireless with receiver)
- **PCM5102A I2S DAC** module (recommended) or SGTL5000 audio codec
- **USB OTG adapter** (if using USB Host cable)
- **Breadboard and jumper wires** for prototyping

## Installation

### 1. Install PlatformIO

#### Option A: Command Line (Recommended)
```bash
# Install PlatformIO Core
python -m pip install --upgrade platformio

# Verify installation
pio --version
```

#### Option B: VS Code Extension
1. Install VS Code
2. Install PlatformIO IDE extension from marketplace
3. Restart VS Code

### 2. Clone the Repository
```bash
git clone https://github.com/Moonwolf711/guitar-hero-teensy-k612-synth.git
cd guitar-hero-teensy-k612-synth
```

## Building the Firmware

### Teensy 4.1 Main Firmware

1. Navigate to Teensy firmware directory:
```bash
cd firmware/teensy-main
```

2. Install dependencies:
```bash
pio lib install
```

3. Build the firmware:
```bash
# Build for Teensy 4.1
pio run -e teensy41

# Or build and upload directly
pio run -e teensy41 --target upload
```

4. Upload to Teensy:
```bash
# Upload firmware
pio run -e teensy41 --target upload

# Monitor serial output
pio device monitor -b 115200
```

### ESP8266 WiFi Module Firmware

1. Navigate to ESP8266 firmware directory:
```bash
cd firmware/esp8266-wifi
```

2. Build the firmware:
```bash
# Build for ESP8266-12E
pio run -e esp12e
```

3. Upload to ESP8266:
```bash
# Upload firmware
pio run -e esp12e --target upload

# Upload filesystem (web interface)
pio run -e esp12e --target uploadfs
```

## Hardware Connections

### Teensy 4.1 Pinout

#### PCM5102A I2S DAC Connection
| PCM5102A Pin | Teensy 4.1 Pin | Description |
|--------------|----------------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| BCK | Pin 21 | Bit clock (BCLK1) |
| DIN | Pin 7 | Data (OUT1A) |
| LCK | Pin 20 | Word clock (LRCLK1) |
| FMT | GND | I2S format select |
| XMT | 3.3V | Soft mute (high = unmuted) |
| SCL | GND | No I2C needed |
| FLT | GND | Normal filter |
| DMP | GND | De-emphasis off |

#### ESP8266 Serial Connection
| ESP8266 Pin | Teensy 4.1 Pin | Description |
|-------------|----------------|-------------|
| TX | Pin 0 (RX1) | Serial receive |
| RX | Pin 1 (TX1) | Serial transmit |
| GND | GND | Common ground |
| VCC | 3.3V | Power (3.3V only!) |
| EN | 3.3V | Chip enable |
| RST | Pin 2 (optional) | Reset control |

#### USB Host Connection
- Connect USB OTG adapter to Teensy 4.1 USB Host port (the one next to Ethernet)
- Plug Xbox 360 Guitar Hero controller into USB adapter

### Alternative: SGTL5000 Audio Shield
If using Teensy Audio Shield with SGTL5000:
1. Stack audio shield on top of Teensy 4.1
2. No additional wiring needed for audio
3. Uncomment `#define USE_SGTL5000` in config.h

## Testing the Firmware

### 1. Test Controller Detection
```bash
# Upload test code
cd firmware/teensy-main
pio run -e teensy41 --target upload -c test/test_controller.cpp

# Monitor output
pio device monitor -b 115200
```

Expected output:
```
Guitar Hero Controller Test
Starting USB Host...
*** Device 0 - VID: 0x1430 PID: 0x4748 - Guitar Hero Controller DETECTED!
```

### 2. Test Audio Output
```bash
# Upload audio test
pio run -e teensy41 --target upload -c test/test_audio.cpp

# You should hear a repeating C-E-G-C chord sequence
```

### 3. Test Scale Quantization
```bash
# Upload scale test
pio run -e teensy41 --target upload -c test/test_scales.cpp

# Interactive test via serial monitor
```

## Configuration

### Modifying Audio Settings
Edit `firmware/teensy-main/include/config.h`:
```cpp
// Change audio quality
#define AUDIO_SAMPLE_RATE 48000  // Higher sample rate

// Adjust polyphony
#define NUM_VOICES 8  // More simultaneous notes

// Change default scale
#define DEFAULT_SCALE SCALE_DORIAN
```

### WiFi Configuration
Edit ESP8266 firmware for custom network:
```cpp
// In esp8266-wifi/src/main.cpp
const char* AP_SSID = "YourNetworkName";
const char* AP_PASS = "YourPassword";
```

## Troubleshooting

### Common Issues

#### 1. Controller Not Detected
- Verify USB Host cable is connected properly
- Check controller is Xbox 360 compatible
- Try different USB port or cable
- Enable debug output: `#define DEBUG_USB_HOST 1`

#### 2. No Audio Output
- Verify I2S connections (especially LRCLK and BCLK)
- Check PCM5102A power LED is on
- Test with simple sine wave generator
- Monitor CPU usage (should be <80%)

#### 3. High CPU Usage
- Reduce number of voices
- Disable effects (reverb/delay)
- Lower sample rate to 22050
- Use simpler waveforms (sine instead of sawtooth)

#### 4. ESP8266 Not Responding
- Check serial connections (TX to RX, RX to TX)
- Verify 3.3V power (NOT 5V!)
- Check baud rate matches (115200)
- Reset ESP8266 manually

### Debug Serial Commands

Connect via serial monitor (115200 baud) and use:
- `s` - Show current status
- `0-5` - Select scale (0=Pentatonic, 1=Minor, etc.)
- `r` - Reset to defaults
- `p` - Performance metrics

## Performance Optimization

### CPU Usage Targets
- Idle: <5%
- Single note: <15%
- Full polyphony (6 voices): <50%
- With effects: <70%
- Maximum safe: 80%

### Memory Usage
- Audio blocks: 64 allocated, typically using 20-30
- RAM usage: ~200KB of 1MB available
- Keep headroom for dynamic allocation

### Latency Optimization
1. Use `TEENSY_OPT_FASTEST_LTO` build flag
2. Set CPU to 600MHz
3. Use smaller audio blocks (64 samples)
4. Disable unnecessary debug output
5. Keep loop() under 1ms execution time

## Advanced Modifications

### Adding Custom Scales
In `scale_quantizer.cpp`:
```cpp
// Add to scaleIntervals array
{0, 2, 4, 6, 8, 10, 11},  // Your custom scale

// Update scaleNames
"Custom Scale",

// Increment NUM_SCALES in header
```

### Custom Waveforms
```cpp
// In main.cpp, setup()
// Create custom waveform table
int16_t customWave[256];
for(int i=0; i<256; i++) {
    customWave[i] = /* your formula */;
}
voice1.arbitraryWaveform(customWave, 172.0);
```

### Adding MIDI Output
```cpp
// Enable USB MIDI in platformio.ini
build_flags = -D USB_MIDI_AUDIO_SERIAL

// Send MIDI notes
usbMIDI.sendNoteOn(note, velocity, channel);
```

## Production Deployment

### Recommended Hardware Setup
1. Use PCB instead of breadboard
2. Add bypass capacitors (100nF) near power pins
3. Use shielded cable for audio output
4. Add ferrite beads on USB cables
5. Include power supply filtering

### Enclosure Considerations
- Ensure adequate ventilation for Teensy
- Use metal enclosure for RF shielding
- Mount PCM5102A away from power supply
- Provide strain relief for cables

## Support and Resources

### Documentation
- [Teensy Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html)
- [USBHost_t36 Documentation](https://github.com/PaulStoffregen/USBHost_t36)
- [PlatformIO Teensy Platform](https://docs.platformio.org/en/latest/platforms/teensy.html)

### Community
- GitHub Issues: [Project Repository](https://github.com/Moonwolf711/guitar-hero-teensy-k612-synth)
- PJRC Forum: [forum.pjrc.com](https://forum.pjrc.com)
- PlatformIO Community: [community.platformio.org](https://community.platformio.org)

### Hardware Suppliers
- Teensy 4.1: [PJRC Store](https://www.pjrc.com/store/teensy41.html)
- PCM5102A DAC: Amazon, AliExpress, Adafruit
- ESP8266 NodeMCU: Amazon, AliExpress
- Guitar Hero Controllers: eBay, local game stores

## License
This project is open source under the MIT License. See LICENSE file for details.