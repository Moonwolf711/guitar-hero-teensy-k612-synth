# AI Context File: Guitar Hero → Teensy K612 Synthesizer

**Project Repository**: `/home/moon_wolf/guitar-hero-teensy-k612-synth`

## Project Overview

Transform a Guitar Hero controller into a standalone, expressive synthesizer using Teensy 4.x microcontroller with integrated ESP8266 WiFi control and OSC broadcasting capabilities. The system creates a complete musical instrument with scale quantization, pitch bend, and multiple synthesis voices.

### Core Technology Stack
- **Teensy 4.x**: Main audio processor (ARM Cortex-M7, USB Host, Audio Library)
- **ESP8266 (ESP-12E)**: WiFi control, web interface, OSC broadcasting
- **Guitar Hero Controller**: HID input device via USB Host
- **Integration Target**: K612 synthesizer (method TBD)

---

## Project State

### Current Workflow Phase
- [x] Phase 1: System Architecture & Design (85% complete)
  - [x] Hardware architecture defined
  - [x] Software architecture designed
  - [x] Control mappings specified
  - [x] Audio processing chain designed
  - [x] WiFi/OSC protocol defined
  - [x] Integration options documented
  - [ ] K612 integration method confirmed
- [ ] Phase 2: Firmware Implementation (0% complete)
  - [ ] Teensy USB Host code
  - [ ] Teensy audio engine
  - [ ] Scale quantization implementation
  - [ ] ESP8266 WiFi firmware
  - [ ] Serial communication protocol
- [ ] Phase 3: Hardware Assembly (0% complete)
  - [ ] Component sourcing
  - [ ] Breadboard prototype
  - [ ] Wiring and connections
  - [ ] Testing and verification
- [ ] Phase 4: Testing & Optimization (0% complete)
- [ ] Phase 5: Documentation (0% complete)

### Session History

#### Session 1: October 30, 2025 - Architecture & Design
**Accomplishments**:
- Created three specialized AI agents (teensy-embedded-synth, session-documenter, research-specialist)
- Designed complete dual-processor architecture (Teensy + ESP8266)
- Mapped all controller inputs from gh_controller_2_.maxpat analysis
- Specified audio processing chain with 6-voice polyphony
- Designed WiFi control interface with web UI and OSC
- Documented three K612 integration options

**Key Decisions**:
- Dual-processor architecture (Teensy for audio + ESP8266 for WiFi)
- 6 musical scales with Minus+Fret selection
- 44.1kHz, 16-bit audio, <5ms latency target
- PlatformIO as primary development environment

**Status**: Architecture complete at 85%. Ready for firmware implementation.

**Next Session**: Begin Teensy firmware implementation (USB Host and basic audio engine)

---

## Key Decisions & Context

### Idea & Validation

#### Project Genesis
- Transform Guitar Hero controller into expressive musical instrument
- Leverage existing Max/MSP implementation (gh_controller_2_.maxpat) for proven control mappings
- Create standalone hardware instrument (no computer required)
- Enable WiFi control for DAW integration and remote parameter control

#### Target Users
- Electronic musicians seeking novel controllers
- Live performers wanting expressive, familiar interfaces
- DIY music technology enthusiasts
- Makers interested in embedded audio systems

#### Success Criteria
- Latency <5ms (imperceptible to performer)
- Reliable USB Host controller detection
- Stable WiFi connectivity and OSC broadcasting
- Musical expression through scale quantization and pitch bend
- Integration with existing K612 synthesizer

### Research Insights

#### Technology Selection Research

**Teensy 4.x vs ESP32 (for main processor)**:
- Decision: Teensy 4.x selected
- Reasoning:
  - Mature Teensy Audio Library optimized for real-time DSP
  - Superior USB Host support for HID controllers
  - Well-documented, stable platform
  - Active community and extensive examples
  - Can integrate with existing K612 codebase

**Single vs Dual Processor**:
- Decision: Dual processor (Teensy + ESP8266)
- Reasoning:
  - Separates real-time audio (Teensy) from network I/O (ESP8266)
  - Teensy focuses on <5ms latency audio processing
  - ESP8266 handles WiFi without affecting audio timing
  - Easier debugging with isolated subsystems
  - More robust than single-processor multitasking

**Scale Quantization System**:
- Decision: 6 pre-defined musical scales
- Scales chosen:
  1. Pentatonic Minor (universal, beginner-friendly)
  2. Natural Minor (common, emotional)
  3. Dorian (jazz, modal)
  4. Hungarian Minor (exotic, dramatic)
  5. Harmonic Minor (classical, tension)
  6. Phrygian (dark, Spanish flavor)
- Reasoning: Based on successful Max/MSP implementation, covers diverse musical styles

**Audio Specifications**:
- 44.1kHz sample rate (standard audio quality)
- 16-bit resolution (matches Teensy Audio Library native format)
- 6-voice polyphony (covers 5 frets + transitions)
- <5ms total latency (imperceptible to performers)
- CPU budget <80% (leaves headroom for stability)

#### Component Selection

**Teensy 4.0 vs 4.1**:
- Both supported
- 4.1 advantages: SD card slot (for sample playback), more pins
- 4.0 advantages: Lower cost, more compact
- Recommendation: Start with 4.1 for flexibility

**Audio Output Options**:
1. SGTL5000 codec (recommended): Line/headphone out, high quality
2. PCM5102 DAC: Line out only, lower cost
3. Teensy built-in DAC: Lowest quality, prototyping only

**ESP8266 vs ESP32**:
- Decision: ESP8266 (ESP-12E module)
- Reasoning: Sufficient for web server + OSC, lower cost, simpler integration
- ESP32 could be future upgrade for Bluetooth support

### Creative Strategy

#### Control Mapping Philosophy
- **Fret Buttons**: Note triggers with scale quantization (removes "wrong notes")
- **Whammy Bar**: Pitch bend for expressive control (guitar-like vibrato)
- **Star Power**: Octave boost (instant register shift)
- **Minus + Fret**: Scale selection (creative mode switching)
- **D-Pad**: Transport and tone control (accessible modifier)
- **Pickup Selector**: Tone presets (familiar to guitarists)

#### Musical Expression Strategy
- Scale quantization enables non-musicians to play musically
- Pitch bend adds human expression (removes "robotic" feel)
- Octave boost expands range without changing hand position
- Multiple scales enable diverse musical styles within one performance

#### Performance Workflow
1. Select scale (Minus + Fret) for musical context
2. Play notes with fret buttons (always in-key)
3. Add expression with whammy bar (pitch bend)
4. Toggle octaves with Star Power for register changes
5. Switch tones with pickup selector for variety
6. Control tempo/transport with D-Pad

### Production Notes

#### Development Environment Setup
**PlatformIO (Recommended)**:
```ini
[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
lib_deps =
    USBHost_t36
    Audio
    MIDI Library
```

**Arduino IDE (Alternative)**:
- Install Teensyduino add-on
- Manually install required libraries
- Select board: Tools → Board → Teensy 4.1

#### Hardware Wiring Notes
**Teensy USB Host Connection**:
- D+ (USB) → Pin 34 (Teensy)
- D- (USB) → Pin 33 (Teensy)
- 5V and GND to USB power
- Requires USB Host cable or adapter

**Teensy ↔ ESP8266 UART**:
- Teensy TX (Pin 1) → Level Shifter → ESP8266 RX
- Teensy RX (Pin 0) ← Level Shifter ← ESP8266 TX
- Level shifter required: Teensy is 3.3V tolerant but ESP8266 is 3.3V only
- Shared GND between Teensy and ESP8266

**I2S Audio Output (SGTL5000)**:
- LRCLK (Pin 20), BCLK (Pin 21), TX (Pin 7), MCLK (Pin 23)
- 3.3V power and GND
- Line out and headphone out available

**Power Distribution**:
- Teensy: 5V via USB (up to 500mA)
- ESP8266: 3.3V via regulator (200-300mA peak)
- Total: Recommend external 5V 2A supply for stability

#### Firmware Architecture Notes

**Teensy Main Loop Structure**:
```cpp
void loop() {
  // USB Host service (non-blocking)
  usb_host.Task();

  // Check for controller updates
  if (controller.available()) {
    processControllerInput();
  }

  // Handle ESP8266 commands
  if (Serial1.available()) {
    processESPCommand();
  }

  // Audio processing happens in Audio Library callbacks
  // Keep loop() fast and non-blocking
}
```

**Audio Processing Strategy**:
- Use Teensy Audio Library objects (no custom audio callbacks)
- Connect objects with patch cables (AudioConnection)
- Update parameters from main loop, not in audio interrupts
- Monitor CPU usage: AudioProcessorUsageMax()

**ESP8266 Serial Protocol**:
- JSON-based messages for simplicity
- Example: `{"cmd":"filter","cutoff":64,"resonance":32}`
- Checksums optional for reliability
- 115200 baud rate (fast enough, well-supported)

#### K612 Integration Options

**Option 1: Parallel MIDI Controller** (Easiest)
- Guitar Hero synth sends MIDI to K612
- K612 remains standalone
- Both can be played simultaneously
- Requires K612 to accept MIDI input

**Option 2: CV/Gate Interface** (Hardware)
- Add DAC for CV output (pitch voltage)
- Add gate output for note triggers
- Integrate with K612 if it accepts CV/Gate
- Requires additional hardware

**Option 3: Direct Code Integration** (Most Integrated)
- Merge Guitar Hero code into K612 firmware
- Share audio processing and synthesis
- Requires K612 source code access
- Most flexible but most complex

#### Testing Strategy
1. **USB Host Testing**: Verify controller detection and HID parsing
2. **Audio Engine Testing**: Verify synthesis, polyphony, latency
3. **Scale Quantization Testing**: Verify all 6 scales work correctly
4. **WiFi Testing**: Verify web UI and OSC broadcasting
5. **Integration Testing**: Verify K612 connection and control
6. **Performance Testing**: Measure CPU usage, latency, stability

#### Known Pitfalls
- USB Host incompatibility with some Guitar Hero models (test early)
- Audio glitches if CPU usage exceeds 80% (optimize aggressively)
- WiFi instability if ESP8266 power supply is noisy (use good regulator)
- Latency accumulation from multiple processing stages (measure at each stage)
- HID report format varies between controller models (implement flexible parser)

---

## Working Instructions

### Current Focus
**Next Session Priority**: Begin Teensy firmware implementation
1. Set up PlatformIO project for Teensy 4.x
2. Implement USB Host initialization and controller detection
3. Create basic audio engine with Teensy Audio Library
4. Test controller input → audio output signal flow

### Active Tasks
- [ ] Create Teensy firmware skeleton (platformio.ini + main.cpp)
- [ ] Implement USB Host controller interface
- [ ] Build core audio engine (6 voices, mixer, effects)
- [ ] Implement scale quantization engine
- [ ] Create ESP8266 communication protocol

### Deferred Features
- Web interface styling and polish
- Preset save/load functionality
- Advanced effects (chorus, flanger)
- Physical enclosure design
- PCB design (breadboard prototype first)

### Critical Questions for User
1. **K612 Synthesizer Details**: What is K612? Custom project or commercial product? What interface options does it have?
2. **K612 Integration Method**: Which of the three options do you prefer? (MIDI, CV/Gate, direct code)
3. **Hardware Preferences**: Teensy 4.0 or 4.1? Audio output type? Enclosure requirements?
4. **Guitar Hero Controller Model**: Which controller do you have? (PS3, Xbox 360, Wii?)
5. **Development Environment**: PlatformIO or Arduino IDE? What OS?

---

## File Structure

```
/home/moon_wolf/guitar-hero-teensy-k612-synth/
├── CLAUDE.md                          # This file (AI context for Claude)
├── AGENTS.md                          # AI context for Agents
├── GEMINI.md                          # AI context for Gemini
├── README.md                          # Project documentation
├── GIT_REMOTE                         # Git remote metadata
├── docs/
│   ├── session-summary-2025-10-30.md  # Detailed session summary
│   ├── hardware-wiring.md             # (To be created)
│   ├── firmware-architecture.md       # (To be created)
│   └── troubleshooting.md             # (To be created)
├── firmware/
│   ├── teensy-main/                   # (To be created)
│   │   ├── src/
│   │   │   └── main.cpp
│   │   ├── include/
│   │   └── platformio.ini
│   └── esp8266-wifi/                  # (To be created)
│       ├── src/
│       │   └── main.cpp
│       └── platformio.ini
├── hardware/
│   ├── schematics/                    # (To be created)
│   └── bom.md                         # (To be created)
└── web-interface/                     # (To be created)
    ├── index.html
    ├── style.css
    └── script.js
```

---

## Quick Reference

### System Specifications
- **Audio**: 44.1kHz, 16-bit, <5ms latency
- **Polyphony**: 6 voices
- **CPU Budget**: <80% (Teensy 4.x)
- **Power**: 5V @ 2A recommended
- **Connectivity**: WiFi 2.4GHz, OSC over UDP port 8000

### Control Mappings Quick Reference
- **Frets (Green/Red/Yellow/Blue/Orange)**: Note triggers
- **Whammy Bar**: Pitch bend (-2 to +2 semitones)
- **Star Power**: Octave boost toggle
- **Minus + Fret**: Scale selection (6 scales)
- **D-Pad**: Up/Down = Tempo, Left/Right = Tone
- **Plus**: Transport start/stop
- **Pickup Selector**: Tone presets (bright/balanced/warm)

### Library Dependencies
- **Teensy**: USBHost_t36, Audio, MIDI Library
- **ESP8266**: ESP8266WiFi, ESP8266WebServer, OSC

### Key File Paths
- **Session Summary**: `/home/moon_wolf/guitar-hero-teensy-k612-synth/docs/session-summary-2025-10-30.md`
- **Agent Definitions**: `/home/moon_wolf/.claude/agents/teensy-embedded-synth.md`
- **Project Root**: `/home/moon_wolf/guitar-hero-teensy-k612-synth/`

---

## Version History

### v0.1.0 - October 30, 2025 (Current)
- Initial architecture and design phase complete
- System specifications defined
- Control mappings documented
- Three AI agents created
- Ready for firmware implementation

---

**Last Updated**: October 30, 2025
**Phase**: Architecture & Design (85% complete)
**Next Milestone**: Firmware implementation Phase 2
**Estimated Completion**: 8-14 sessions (40-70 hours)
