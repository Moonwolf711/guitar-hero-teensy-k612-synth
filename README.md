# Guitar Hero → Teensy K612 Synthesizer

Transform a Guitar Hero controller into a standalone, expressive musical synthesizer using Teensy 4.x and ESP8266.

## Overview

This project converts a Guitar Hero game controller into a professional-quality musical instrument with:
- 6-voice polyphonic synthesis
- Musical scale quantization (6 scales)
- Real-time pitch bend control via whammy bar
- WiFi-enabled web interface
- OSC message broadcasting for DAW integration
- Integration with K612 synthesizer

### Key Features

- **Low Latency**: <5ms total system latency for real-time performance
- **Expressive Control**: Whammy bar pitch bend, octave shifting, dynamic tone control
- **Musical Intelligence**: Scale quantization ensures all notes are musically correct
- **WiFi Control**: Web-based parameter control from any device
- **DAW Integration**: OSC protocol support for Ableton, Max/MSP, TouchOSC
- **Standalone Operation**: No computer required after initial programming

## System Architecture

```
┌─────────────────────┐
│  Guitar Hero        │
│  Controller (USB)   │
└──────────┬──────────┘
           │ USB HID
┌──────────▼──────────┐         ┌─────────────────┐
│                     │ UART    │                 │
│   Teensy 4.x        ├────────►│  ESP8266        │
│   (Audio + USB)     │         │  (WiFi)         │
│                     │         │                 │
└──────────┬──────────┘         └────────┬────────┘
           │ I2S                         │
┌──────────▼──────────┐         ┌───────▼─────────┐
│  Audio Codec/DAC    │         │  Web Interface  │
│  (SGTL5000/PCM5102) │         │  OSC Broadcast  │
└─────────────────────┘         └─────────────────┘
```

### Hardware Components

**Required:**
- Teensy 4.0 or 4.1 microcontroller ($25-30)
- ESP8266 ESP-12E module ($5)
- Guitar Hero controller (PS3/Xbox 360 compatible, $10-30)
- Audio codec: SGTL5000 or PCM5102 DAC ($15)
- USB Host cable/adapter
- 3.3V voltage regulator for ESP8266
- Level shifter for UART communication
- Breadboard and jumper wires (for prototyping)

**Optional:**
- Enclosure
- Custom PCB
- Power supply (5V 2A recommended)

**Estimated Total Cost**: $80-100

## Control Mappings

### Primary Controls
| Control | Function | Details |
|---------|----------|---------|
| **Green Fret** | Note trigger | Quantized to selected scale |
| **Red Fret** | Note trigger | Quantized to selected scale |
| **Yellow Fret** | Note trigger | Quantized to selected scale |
| **Blue Fret** | Note trigger | Quantized to selected scale |
| **Orange Fret** | Note trigger | Quantized to selected scale |
| **Whammy Bar** | Pitch bend | -2 to +2 semitones, continuous control |
| **Star Power** | Octave boost | Toggle +1 octave |

### Scale Selection (Minus + Fret)
| Button Combination | Scale |
|-------------------|-------|
| Minus + Green | Pentatonic Minor |
| Minus + Red | Natural Minor |
| Minus + Yellow | Dorian |
| Minus + Blue | Hungarian Minor |
| Minus + Orange | Harmonic Minor |
| Minus (hold alone) | Phrygian |

### Additional Controls
| Control | Function |
|---------|----------|
| **D-Pad Up** | Increase tempo |
| **D-Pad Down** | Decrease tempo |
| **D-Pad Left** | Previous tone/pattern |
| **D-Pad Right** | Next tone/pattern |
| **Plus Button** | Transport start/stop |
| **Pickup Selector** | Tone preset (bright/balanced/warm) |

## Audio Specifications

- **Sample Rate**: 44.1kHz
- **Bit Depth**: 16-bit
- **Polyphony**: 6 voices
- **Latency**: <5ms (target)
- **CPU Usage**: <80% (Teensy 4.x)
- **Output**: Line level or headphone

### Audio Processing Chain

1. **Input**: USB Host → HID parsing → Button/analog processing
2. **Scale Quantization**: Map fret buttons to scale notes
3. **Synthesis Engine**:
   - 6x Waveform generators (saw, square, triangle, sine)
   - 6x ADSR envelope generators
   - Voice mixer
4. **Effects**:
   - Multi-mode filter (LP/HP/BP)
   - Reverb
   - Delay
   - Optional: Chorus
5. **Output**: I2S → Audio codec → Line/headphone out

## Software Architecture

### Firmware Structure
```
firmware/
├── teensy-main/              # Teensy 4.x firmware
│   ├── src/
│   │   ├── main.cpp          # Main loop and setup
│   │   ├── usb_host_controller.cpp  # Guitar Hero HID interface
│   │   ├── audio_engine.cpp  # Synthesis and DSP
│   │   ├── midi_handler.cpp  # MIDI I/O
│   │   └── esp_communication.cpp    # ESP8266 serial protocol
│   ├── include/
│   └── platformio.ini        # PlatformIO configuration
└── esp8266-wifi/             # ESP8266 firmware
    ├── src/
    │   ├── main.cpp          # WiFi setup and main loop
    │   ├── web_server.cpp    # HTTP web interface
    │   ├── osc_handler.cpp   # OSC message broadcasting
    │   └── serial_protocol.cpp  # Teensy communication
    └── platformio.ini
```

### Libraries Required

**Teensy 4.x:**
- USBHost_t36 (USB Host support)
- Audio (Teensy Audio Library)
- MIDI Library

**ESP8266:**
- ESP8266WiFi
- ESP8266WebServer
- OSC (for OSC protocol)

## Getting Started

### Prerequisites

1. **Development Environment**: Choose one:
   - **PlatformIO** (recommended): Professional toolchain with dependency management
   - **Arduino IDE**: Install Teensyduino add-on

2. **Hardware**: Source all required components (see Hardware Components section)

3. **Tools**:
   - Soldering iron (for permanent assembly)
   - Multimeter (for debugging)
   - Breadboard and jumper wires (for prototyping)

### Installation

#### Option 1: PlatformIO (Recommended)

```bash
# Install PlatformIO Core
pip install platformio

# Clone repository
git clone <repository-url>
cd guitar-hero-teensy-k612-synth

# Build and upload Teensy firmware
cd firmware/teensy-main
pio run --target upload

# Build and upload ESP8266 firmware
cd ../esp8266-wifi
pio run --target upload
```

#### Option 2: Arduino IDE

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [Teensyduino](https://www.pjrc.com/teensy/td_download.html)
3. Install required libraries via Library Manager
4. Open `firmware/teensy-main/src/main.cpp`
5. Select board: Tools → Board → Teensy 4.1
6. Upload to Teensy
7. Repeat for ESP8266 firmware

### Hardware Assembly

#### Wiring Guide

**USB Host Connection (Guitar Hero Controller):**
```
Guitar Hero USB Connector:
  D+ (green)  →  Teensy Pin 34
  D- (white)  →  Teensy Pin 33
  5V (red)    →  5V power supply
  GND (black) →  Common ground
```

**Teensy ↔ ESP8266 UART:**
```
Teensy TX (Pin 1)  →  Level Shifter (5V→3.3V)  →  ESP8266 RX
Teensy RX (Pin 0)  ←  Level Shifter (3.3V→5V)  ←  ESP8266 TX
Teensy GND         →  ESP8266 GND (common ground)
```

**I2S Audio (SGTL5000 codec):**
```
Teensy Pin 20 (LRCLK)  →  SGTL5000 LRCLK
Teensy Pin 21 (BCLK)   →  SGTL5000 BCLK
Teensy Pin 7  (TX)     →  SGTL5000 RX
Teensy Pin 23 (MCLK)   →  SGTL5000 MCLK
3.3V                   →  SGTL5000 VCC
GND                    →  SGTL5000 GND
```

**Power Distribution:**
```
5V Power Supply (2A)  →  Teensy 5V pin
                      →  5V rail (for peripherals)
3.3V Regulator Input  ←  5V rail
3.3V Regulator Output →  ESP8266 VCC (3.3V @ 300mA)
```

### First Boot

1. Connect Guitar Hero controller to Teensy USB Host
2. Power on system (via USB or external 5V supply)
3. Teensy should detect controller (LED indication)
4. ESP8266 creates WiFi access point: "GuitarHeroSynth"
5. Connect to WiFi (password: "guitarhero")
6. Open browser to `http://192.168.4.1` for web interface
7. Test fret buttons → should hear synthesized notes

## WiFi Configuration

### Access Point Mode (Default)
- SSID: `GuitarHeroSynth`
- Password: `guitarhero`
- IP Address: `192.168.4.1`

### Station Mode (Connect to Existing WiFi)
1. Connect to access point
2. Open web interface
3. Navigate to WiFi settings
4. Enter your WiFi SSID and password
5. Reboot - device will connect to your network

## OSC Protocol

OSC messages are broadcast on UDP port 8000.

### Message Format

**Controller State:**
```
/guitar/fret/green [0-1]          # Fret button state
/guitar/fret/red [0-1]
/guitar/fret/yellow [0-1]
/guitar/fret/blue [0-1]
/guitar/fret/orange [0-1]
/guitar/whammy [0.0-1.0]          # Whammy bar position
/guitar/starpower [0-1]           # Star power button
/guitar/scale [0-5]               # Current scale (0-5)
/guitar/pickup [0-2]              # Pickup selector position
```

**Synthesizer State:**
```
/synth/voice/[1-6]/note [0-127]   # MIDI note number
/synth/voice/[1-6]/velocity [0-127]
/synth/voice/[1-6]/pitchbend [-1.0-1.0]
/synth/filter/cutoff [0-127]
/synth/filter/resonance [0-127]
/synth/effects/reverb/mix [0.0-1.0]
/synth/effects/delay/time [0-1000]  # milliseconds
```

### OSC Control (Incoming)

Send OSC messages to device IP on port 8000 to control parameters:
```
/synth/filter/cutoff [0-127]      # Set filter cutoff
/synth/effects/reverb/mix [0-1]   # Set reverb mix
/synth/preset/load [0-9]          # Load preset
```

## K612 Integration

Three integration options are supported:

### Option 1: Parallel MIDI Controller (Easiest)
- Guitar Hero synth outputs MIDI to K612
- Both instruments remain standalone
- Can play simultaneously or separately
- **Requirements**: K612 must accept MIDI input

### Option 2: CV/Gate Interface (Hardware)
- Add CV/Gate output to Guitar Hero synth
- Connect to K612 CV/Gate inputs
- **Requirements**: K612 must have CV/Gate inputs, additional DAC hardware

### Option 3: Direct Code Integration (Most Flexible)
- Merge Guitar Hero firmware with K612 codebase
- Share synthesis engine and audio processing
- **Requirements**: Access to K612 source code

**Note**: Integration method depends on K612 capabilities. See K612 documentation for compatibility.

## Troubleshooting

### Controller Not Detected
- **Check**: USB Host wiring (D+, D-, 5V, GND)
- **Check**: 5V power supply is adequate (500mA minimum)
- **Test**: Try different Guitar Hero controller model
- **Debug**: Monitor Serial output for USB Host enumeration messages

### No Audio Output
- **Check**: I2S wiring (LRCLK, BCLK, TX, MCLK)
- **Check**: Audio codec is powered (3.3V)
- **Test**: Use Serial.print to verify audio objects are initialized
- **Debug**: Monitor AudioProcessorUsage() - should be <80%

### Audio Glitches/Dropouts
- **Cause**: CPU usage too high (>80%)
- **Solution**: Reduce polyphony, disable effects, optimize code
- **Solution**: Increase audio buffer size (trades latency for stability)
- **Debug**: Use AudioProcessorUsageMax() to identify peak usage

### WiFi Connection Drops
- **Check**: ESP8266 power supply is stable (3.3V, 300mA)
- **Check**: Good quality voltage regulator (noisy power causes WiFi issues)
- **Solution**: Add decoupling capacitors (100uF + 0.1uF) near ESP8266
- **Solution**: Implement reconnection logic in firmware

### High Latency
- **Measure**: Add timing code to measure each processing stage
- **Solution**: Reduce audio buffer size (increases CPU usage)
- **Solution**: Optimize audio processing chain (remove unused effects)
- **Target**: <5ms total latency is imperceptible

### MIDI Not Working
- **Check**: MIDI library is installed
- **Check**: Correct MIDI channel selected
- **Test**: Use MIDI monitor software to verify output
- **Debug**: Send test MIDI messages from Serial console

## Performance Optimization

### CPU Usage Guidelines
- **Target**: <30% average, <80% peak
- **Monitor**: Use `AudioProcessorUsageMax()` regularly
- **Optimize**:
  - Static memory allocation (no malloc in audio callback)
  - Fixed-point arithmetic for filters/envelopes
  - Efficient scale lookup tables
  - Voice stealing for polyphony management

### Memory Management
- **Teensy 4.1**: 1024KB RAM available
- **Audio buffers**: ~50KB (256 samples @ 44.1kHz)
- **Code + data**: ~200KB
- **Remaining**: ~774KB for user code and variables

### Latency Breakdown (Target <5ms)
- USB Host polling: ~1ms
- Audio buffer processing: ~3ms (256 samples @ 44.1kHz)
- ESP8266 communication: <1ms
- **Total**: ~5ms

## Development Resources

### Documentation
- [Teensy 4.x Documentation](https://www.pjrc.com/teensy/)
- [Teensy Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html)
- [USBHost_t36 Library](https://github.com/PaulStoffregen/USBHost_t36)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)

### Community
- [PJRC Forum](https://forum.pjrc.com/) - Teensy support
- [ESP8266 Community](https://www.esp8266.com/) - ESP8266 support
- [Arduino Forum](https://forum.arduino.cc/) - General Arduino support

### Related Projects
- [Teensy Audio Library Examples](https://www.pjrc.com/teensy/td_libs_Audio.html)
- [Guitar Hero MIDI Controller Projects](https://github.com/topics/guitar-hero-controller)

## Project Status

- **Current Phase**: Architecture & Design (85% complete)
- **Next Phase**: Firmware Implementation (0% complete)
- **Estimated Completion**: 8-14 sessions (40-70 hours)

### Completed
- [x] System architecture design
- [x] Hardware component selection
- [x] Control mapping specification
- [x] Software architecture design
- [x] Documentation

### In Progress
- [ ] Teensy firmware implementation
- [ ] ESP8266 firmware implementation
- [ ] Web interface development

### Planned
- [ ] Hardware assembly and testing
- [ ] K612 integration
- [ ] Performance optimization
- [ ] User documentation and tutorials

## Contributing

This is currently a personal project. Contributions, suggestions, and feedback are welcome.

## License

MIT License (or specify your chosen license)

## Acknowledgments

- Based on control mappings from Max/MSP patch `gh_controller_2_.maxpat`
- Teensy Audio Library by Paul Stoffregen (PJRC)
- USBHost_t36 Library by Paul Stoffregen
- ESP8266 Arduino Core by ESP8266 Community

## Contact

Project repository: `/home/moon_wolf/guitar-hero-teensy-k612-synth`

Session documentation: `docs/session-summary-2025-10-30.md`

---

**Last Updated**: October 30, 2025
