# Session Summary: October 30, 2025

## Project Overview
**Guitar Hero Controller → Teensy K612 Synthesizer**
Transform a Guitar Hero controller into a standalone, expressive synthesizer using Teensy 4.x microcontroller with integrated ESP8266 WiFi control and OSC broadcasting capabilities.

## Current Phase
**Phase 1: System Architecture & Design** - 85% complete

## Session Accomplishments

### 1. Specialized Agent Creation
Created three purpose-built AI agents for project development:

- **teensy-embedded-synth** (`/home/moon_wolf/.claude/agents/teensy-embedded-synth.md`)
  - Elite embedded systems engineer specializing in Teensy 4.x, audio synthesis, USB Host
  - Expertise in real-time audio DSP, hardware/software integration, and musical instruments
  - Provides complete stack knowledge from hardware interfacing to DSP algorithms

- **session-documenter** (`/home/moon_wolf/.claude/agents/session-documenter.md`)
  - Session documentation specialist ensuring zero information loss between sessions
  - Maintains AI context files (CLAUDE.md, AGENTS.md, GEMINI.md)
  - Manages project state tracking and comprehensive session summaries

- **research-specialist** (`/home/moon_wolf/.claude/agents/research-specialist.md`)
  - Web research and current information gathering
  - Technology evaluation and component sourcing
  - Best practices and implementation pattern research

### 2. Complete System Architecture Design

#### Hardware Architecture
- **Main Controller**: Teensy 4.x (ARM Cortex-M7)
  - USB Host for Guitar Hero controller (HID interface)
  - Teensy Audio Library for synthesis and DSP
  - I2S audio output (SGTL5000 codec or PCM5102 DAC)
  - MIDI input/output capabilities

- **WiFi Module**: ESP8266 (ESP12e)
  - Web interface for parameter control
  - OSC message broadcasting for DAW integration
  - Serial UART communication with Teensy
  - Access point and station modes

- **Integration Target**: K612 synthesizer
  - Three integration options documented:
    1. Parallel MIDI controller
    2. CV/Gate interface
    3. Direct code integration

#### Control Mappings (from gh_controller_2_.maxpat analysis)
**Fret Buttons (Notes):**
- Green Fret → Note trigger with quantization
- Red Fret → Note trigger with quantization
- Yellow Fret → Note trigger with quantization
- Blue Fret → Note trigger with quantization
- Orange Fret → Note trigger with quantization

**Whammy Bar:**
- Continuous control → Pitch bend (-2 to +2 semitones)
- Real-time expression control

**Star Power Button:**
- Toggle → Octave boost (+1 octave)
- Performance modifier

**Minus Button + Fret:**
- Scale selection system (6 scales available)
- Green → Pentatonic Minor
- Red → Natural Minor
- Yellow → Dorian
- Blue → Hungarian Minor
- Orange → Harmonic Minor
- (Default) → Phrygian

**D-Pad Controls:**
- Up → Tempo increase
- Down → Tempo decrease
- Left → Pattern/voice selection
- Right → Pattern/voice selection

**Pickup Selector (3-position):**
- Position 1 → Bright/lead tone
- Position 2 → Balanced tone
- Position 3 → Warm/rhythm tone

**Plus Button:**
- Transport control (start/stop)

### 3. Software Architecture Design

#### Firmware Structure
```
firmware/
├── teensy-main/
│   ├── src/
│   │   ├── main.cpp
│   │   ├── usb_host_controller.cpp
│   │   ├── audio_engine.cpp
│   │   ├── midi_handler.cpp
│   │   └── esp_communication.cpp
│   ├── include/
│   └── platformio.ini
└── esp8266-wifi/
    ├── src/
    │   ├── main.cpp
    │   ├── web_server.cpp
    │   ├── osc_handler.cpp
    │   └── serial_protocol.cpp
    └── platformio.ini
```

#### Audio Processing Chain
1. USB Host reads Guitar Hero HID data
2. Button/analog processing with debouncing
3. Scale quantization engine
4. Pitch bend processing
5. Synthesis engine (6-voice polyphony)
   - Waveform selection (saw, square, triangle, sine)
   - ADSR envelope generators
   - Multi-mode filter (LP, HP, BP)
   - LFO modulation
6. Effects chain
   - Reverb
   - Delay
   - Chorus
7. I2S audio output

#### WiFi Control Interface
- HTTP web server with responsive UI
- Real-time parameter updates
- OSC message broadcasting (UDP port 8000)
- WebSocket for low-latency bidirectional control
- Configuration storage (WiFi credentials, scale settings)

### 4. Documentation Created

#### Technical Specifications
- **Latency Target**: <5ms total system latency
- **Audio Quality**: 44.1kHz sample rate, 16-bit
- **Polyphony**: 6 voices
- **CPU Budget**: <80% on Teensy 4.x
- **Power Requirements**: 5V @ 500mA (USB-powered)

#### Hardware Requirements
**Required Components:**
- Teensy 4.0 or 4.1
- ESP8266 ESP-12E module
- Guitar Hero controller (PS3/Xbox 360 compatible)
- USB Host cable/adapter
- Audio codec (SGTL5000) or DAC (PCM5102)
- 3.3V voltage regulator (for ESP8266)
- Level shifter (for Teensy-ESP UART communication)

**Wiring Connections:**
- Teensy USB Host pins (D+ → Pin 34, D- → Pin 33)
- Teensy-ESP UART (TX → RX with level shift)
- I2S Audio (LRCLK, BCLK, TX, MCLK pins)
- SPI for ESP8266 (if needed)

#### Software Installation Methods
**Option 1: PlatformIO (Recommended)**
- Cross-platform, professional toolchain
- Library management built-in
- Multiple board support
- Configuration in platformio.ini

**Option 2: Arduino IDE**
- Teensyduino add-on required
- Manual library installation
- Beginner-friendly GUI

### 5. OSC Protocol Specification
```
/guitar/fret/green [0-1]
/guitar/fret/red [0-1]
/guitar/fret/yellow [0-1]
/guitar/fret/blue [0-1]
/guitar/fret/orange [0-1]
/guitar/whammy [0.0-1.0]
/guitar/starpower [0-1]
/guitar/scale [0-5]
/guitar/pickup [0-2]
/synth/voice/[1-6]/note [0-127]
/synth/voice/[1-6]/velocity [0-127]
/synth/voice/[1-6]/pitchbend [-1.0-1.0]
/synth/filter/cutoff [0-127]
/synth/filter/resonance [0-127]
/synth/effects/reverb/mix [0.0-1.0]
/synth/effects/delay/time [0-1000ms]
```

### 6. Troubleshooting Guide
**Common Issues & Solutions:**
- USB Host not detecting controller → Check 5V power supply, verify USB pins
- Audio glitches/dropouts → Reduce CPU usage, optimize buffer sizes
- WiFi connection drops → Implement reconnection logic, check antenna
- Latency too high → Reduce audio buffer size, optimize audio chain
- MIDI timing issues → Verify USB MIDI vs Serial MIDI configuration

### 7. Performance Optimization Strategy
- Static memory allocation (no malloc in audio callback)
- Fixed-point arithmetic for filters and envelopes
- DMA for I2S audio transfer
- Interrupt-driven USB Host reading
- Efficient scale quantization lookup tables
- Voice stealing algorithm for polyphony management

## Key Decisions Made

### 1. **Dual-Processor Architecture**
**Decision**: Use Teensy 4.x for audio + ESP8266 for WiFi (rather than single ESP32)
**Rationale**:
- Teensy Audio Library is mature and optimized for real-time audio
- Teensy 4.x has superior USB Host capabilities for HID controllers
- Separates real-time audio (Teensy) from network I/O (ESP8266)
- Easier debugging with isolated subsystems
- Can integrate with existing K612 codebase on Teensy

### 2. **Scale Quantization System**
**Decision**: Implement 6 musical scales with Minus+Fret button selection
**Rationale**:
- Provides musical expression even for non-musicians
- Based on successful Max/MSP implementation (gh_controller_2_.maxpat)
- Scales chosen cover diverse musical styles (minor, dorian, hungarian, etc.)
- Real-time switching enables performance creativity

### 3. **Integration Strategy with K612**
**Decision**: Offer three integration options (parallel MIDI, CV/Gate, direct code)
**Rationale**:
- Flexibility for different K612 configurations
- MIDI option preserves K612 as standalone unit
- CV/Gate enables analog integration
- Direct code integration maximizes feature sharing
- User can choose based on their specific K612 setup

### 4. **WiFi Control Interface**
**Decision**: Implement both web UI and OSC broadcasting
**Rationale**:
- Web UI enables standalone control via smartphone/tablet
- OSC integration allows DAW control (Ableton, Max/MSP, TouchOSC)
- Dual approach serves both live performance and studio workflows
- WebSocket provides low-latency bidirectional communication

### 5. **Development Environment**
**Decision**: Primary support for PlatformIO, secondary for Arduino IDE
**Rationale**:
- PlatformIO offers professional toolchain with dependency management
- Better suited for dual-firmware project (Teensy + ESP8266)
- Arduino IDE support ensures accessibility for beginners
- Documentation covers both environments

### 6. **Audio Quality Parameters**
**Decision**: 44.1kHz, 16-bit, <5ms latency, 6-voice polyphony
**Rationale**:
- 44.1kHz is standard audio quality, sufficient for musical performance
- 16-bit matches Teensy Audio Library native format
- <5ms latency is imperceptible to performers
- 6 voices covers 5 frets + pitch bend transitions
- Leaves CPU headroom for effects processing

## Current Project State

### Completed (85%)
- [x] System architecture design
- [x] Hardware component selection
- [x] Control mapping specification (from Max patch analysis)
- [x] Software architecture design
- [x] Audio processing chain design
- [x] WiFi control interface design
- [x] OSC protocol specification
- [x] Integration options with K612
- [x] Troubleshooting guide
- [x] Performance optimization strategy
- [x] Development environment setup instructions
- [x] Specialized AI agents created

### In Progress (15%)
- [ ] Firmware implementation (Teensy main code)
- [ ] Firmware implementation (ESP8266 WiFi module)
- [ ] Web interface HTML/CSS/JavaScript
- [ ] Hardware assembly and wiring
- [ ] K612 integration code

### Not Started
- [ ] Physical enclosure design
- [ ] PCB design (optional, can prototype with breadboard)
- [ ] Hardware testing and debugging
- [ ] Firmware testing and optimization
- [ ] Performance benchmarking
- [ ] User documentation and tutorials
- [ ] Video demonstration

## Next Steps (Prioritized)

### Phase 2: Firmware Implementation (Next Session Focus)

#### Immediate Next Steps (Session Priority)
1. **Create Teensy Main Firmware Skeleton**
   - Set up PlatformIO project for Teensy 4.x
   - Configure platformio.ini with required libraries:
     - USBHost_t36 (for Guitar Hero controller)
     - Audio (Teensy Audio Library)
     - MIDI (USB and Serial MIDI)
   - Implement basic USB Host initialization
   - Test controller detection and HID report parsing

2. **Implement USB Host Controller Interface**
   - Parse Guitar Hero HID reports (frets, whammy, buttons)
   - Implement debouncing for button inputs
   - Map analog whammy bar to pitch bend range
   - Create debug output for controller state

3. **Build Core Audio Engine**
   - Initialize Teensy Audio Library objects:
     - 6x AudioSynthWaveform (voices)
     - 6x AudioEffectEnvelope (ADSR)
     - AudioMixer4 (voice mixing)
     - AudioFilterStateVariable (multi-mode filter)
     - AudioEffectFreeverb (reverb)
     - AudioEffectDelay (delay)
   - Connect audio routing
   - Implement basic note-on/note-off handlers

4. **Implement Scale Quantization**
   - Create scale definition tables (6 scales)
   - Implement scale selection logic (Minus + Fret)
   - Build note quantization function
   - Test with different scales

5. **Create ESP8266 Communication Protocol**
   - Design serial packet format (JSON or binary)
   - Implement Teensy TX/RX handlers
   - Create parameter update message types
   - Add error handling and checksums

#### Secondary Next Steps (After Core Firmware)
6. **ESP8266 WiFi Firmware**
   - Set up PlatformIO project for ESP8266
   - Implement WiFi station and AP modes
   - Create web server with parameter control UI
   - Implement OSC message broadcasting
   - Build serial bridge to Teensy

7. **Hardware Assembly**
   - Breadboard prototype wiring
   - Test USB Host connection
   - Verify ESP8266 communication
   - Test audio output

8. **K612 Integration**
   - Choose integration method (MIDI, CV/Gate, or direct code)
   - Implement selected interface
   - Test combined system
   - Document integration process

#### Future Sessions
9. **Testing and Optimization**
   - Measure CPU usage and optimize
   - Test latency and reduce if needed
   - Verify polyphony and voice stealing
   - Test all control mappings

10. **Web Interface Polish**
    - Design responsive UI
    - Implement real-time parameter visualization
    - Add preset save/load functionality
    - Test on mobile devices

11. **Documentation and Tutorials**
    - Create assembly guide with photos
    - Write firmware flashing tutorial
    - Document WiFi configuration process
    - Create troubleshooting FAQ

## Technical Challenges & Considerations

### Identified Challenges
1. **USB Host Compatibility**: Not all Guitar Hero controllers use identical HID reports
   - Solution: Implement flexible HID parser with configurable mappings

2. **Audio Latency**: Multiple processing stages can accumulate latency
   - Solution: Measure at each stage, optimize buffer sizes, use DMA

3. **WiFi Stability**: ESP8266 can be temperamental with connectivity
   - Solution: Robust reconnection logic, watchdog timer, status LED

4. **Power Distribution**: USB Host + ESP8266 + Audio codec = significant current draw
   - Solution: External 5V power supply recommended, calculate total current budget

5. **K612 Integration Complexity**: Unknown K612 internal architecture
   - Solution: Three flexible integration options provided

### Risk Mitigation
- Start with minimal viable product (MVP): Just USB Host → Audio synthesis
- Add features incrementally: WiFi, OSC, K612 integration are modular
- Test each subsystem independently before integration
- Maintain fallback modes (e.g., work without WiFi if module fails)

## Resources & References

### Hardware Datasheets
- Teensy 4.x: https://www.pjrc.com/teensy/
- ESP8266: https://www.espressif.com/en/products/socs/esp8266
- SGTL5000 Audio Codec: https://www.nxp.com/products/

### Software Libraries
- USBHost_t36: https://github.com/PaulStoffregen/USBHost_t36
- Teensy Audio Library: https://www.pjrc.com/teensy/td_libs_Audio.html
- ESP8266 Arduino Core: https://github.com/esp8266/Arduino
- OSC Library: https://github.com/CNMAT/OSC

### Reference Projects
- gh_controller_2_.maxpat (Max/MSP implementation - control mappings source)
- K612 synthesizer (integration target - specifics TBD)

### Community Resources
- PJRC Forum (Teensy support)
- ESP8266 Community Forum
- Arduino Audio Forum

## Blockers / Questions for User

### Critical Information Needed
1. **K612 Synthesizer Details**:
   - What is the K612 synthesizer? (Hardware model? Custom project?)
   - Current interface options? (MIDI input? CV/Gate? Direct code access?)
   - Preferred integration method?
   - Do you have access to K612 source code?

2. **Guitar Hero Controller**:
   - Which model? (PS3, Xbox 360, Wii?)
   - Already available for testing?

3. **Hardware Preferences**:
   - Teensy 4.0 or 4.1? (4.1 has SD card, more pins)
   - Audio output preference? (Line out to mixer? Headphone jack? Eurorack CV?)
   - Enclosure requirements? (Desktop unit? Portable? Rack mount?)

4. **Development Environment**:
   - PlatformIO or Arduino IDE preference?
   - Operating system for development?
   - Experience level with Teensy/embedded development?

5. **Feature Priorities**:
   - Which features are must-have vs nice-to-have?
   - WiFi control essential or optional?
   - K612 integration priority level?

### Recommended First Action
**User should clarify K612 synthesizer details** before proceeding with firmware implementation, as this affects integration strategy and code architecture.

## Project Metrics

### Estimated Completion Timeline
- **Phase 1: Architecture & Design**: 85% complete (current session)
- **Phase 2: Firmware Implementation**: 4-6 sessions (20-30 hours)
- **Phase 3: Hardware Assembly**: 1-2 sessions (5-10 hours)
- **Phase 4: Testing & Optimization**: 2-3 sessions (10-15 hours)
- **Phase 5: Documentation**: 1-2 sessions (5-10 hours)

**Total Estimated Time**: 8-14 sessions (40-70 hours)

### Code Metrics (Estimated)
- Teensy firmware: ~1500-2000 lines
- ESP8266 firmware: ~800-1200 lines
- Web interface: ~400-600 lines (HTML/CSS/JS)
- Total: ~2700-3800 lines

### Hardware Cost Estimate
- Teensy 4.1: $30
- ESP8266 ESP-12E: $5
- SGTL5000 audio board: $15
- Guitar Hero controller: $10-30 (used)
- Miscellaneous components: $20
- **Total**: ~$80-100

---

## Session Update: Hardware Configuration Confirmed

### Date: October 30, 2025 (continued session)

### Accomplishments
**Hardware Specifications Finalized**:
1. **Teensy Model Confirmed**: Teensy 4.1
   - ARM Cortex-M7 @ 600MHz
   - 1MB RAM
   - Built-in SD card slot (for sample playback and data logging)
   - Extra GPIO pins for expansion

2. **Controller Model Confirmed**: Xbox 360 Guitar Hero Controller
   - USB HID compatible
   - Well-documented button/analog mappings
   - Widely available and tested

3. **K612 Synthesizer Identified**: Pre-compiled firmware (teensy_k612_standalone_synth_ino.hex)
   - Integration method: Direct code merge (Option 3) planned
   - Will require decompiling or source code access

4. **Development Environment Confirmed**: PlatformIO preferred

### Documentation Updates
- Updated all AI context files (CLAUDE.md, AGENTS.md, GEMINI.md) with confirmed hardware specs
- Updated README.md with specific hardware models in architecture diagram
- Enhanced GIT_REMOTE with detailed manual setup instructions (GitHub CLI not installed)
- Committed changes to git repository

### Git Repository Status
- **Branch**: main
- **Total Commits**: 3
  - Initial setup
  - Documentation status
  - Hardware configuration update (this session)
- **Status**: Clean working tree, ready for remote setup
- **Remote**: Not yet configured (awaiting user GitHub account setup)

### Key Decisions Documented
1. **Teensy 4.1 Selected** - Provides maximum flexibility with SD card and extra pins
2. **Xbox 360 Controller** - Known-good USB HID implementation
3. **K612 Integration** - Direct code merge for tightest integration
4. **PlatformIO** - Professional development environment with dependency management

### Remaining Questions
1. **Audio Output**: SGTL5000 codec (line/headphone) or PCM5102 DAC (line only)?
2. **Enclosure**: Portable handheld unit or desktop/rack mount?
3. **K612 Source Code**: Available or requires decompilation?

### GitHub Repository Setup Required
User needs to manually create GitHub repository:
- **Repository Name**: `guitar-hero-teensy-k612-synth`
- **Instructions**: See `/home/moon_wolf/guitar-hero-teensy-k612-synth/GIT_REMOTE` for detailed steps
- **Options**: GitHub CLI or manual web-based creation
- **Suggested Visibility**: Public (for open-source hardware project)

### Phase Completion Update
- **Phase 1: Architecture & Design**: **90% complete** (↑5% from hardware confirmation)
  - [x] Hardware architecture defined
  - [x] Software architecture designed
  - [x] Control mappings specified
  - [x] Audio processing chain designed
  - [x] WiFi/OSC protocol defined
  - [x] Integration options documented
  - [x] **Hardware models confirmed** ← NEW
  - [ ] K612 integration method finalized (pending source code access)

### Next Immediate Steps
1. User creates GitHub repository (instructions in GIT_REMOTE)
2. Push local repository to GitHub remote
3. Source SGTL5000 audio board or PCM5102 DAC
4. Obtain or verify K612 source code availability
5. Begin Teensy firmware skeleton (platformio.ini + main.cpp structure)

---

## Session Metadata
- **Date**: October 30, 2025
- **Duration**: Extended planning and architecture session + hardware configuration
- **AI Agents Used**: session-documenter (this session), teensy-embedded-synth (created), research-specialist (created)
- **Files Created**:
  - `/home/moon_wolf/.claude/agents/teensy-embedded-synth.md`
  - `/home/moon_wolf/.claude/agents/session-documenter.md`
  - `/home/moon_wolf/.claude/agents/research-specialist.md`
  - `/home/moon_wolf/guitar-hero-teensy-k612-synth/docs/session-summary-2025-10-30.md` (this file)
- **Files Updated** (Session Update):
  - `CLAUDE.md`, `AGENTS.md`, `GEMINI.md` (hardware specs)
  - `README.md` (architecture diagram and hardware list)
  - `GIT_REMOTE` (detailed setup instructions)
- **Git Commits**: 3 (initial + docs + hardware config)
- **Next Session**: GitHub remote setup → Firmware implementation (Teensy main code)
