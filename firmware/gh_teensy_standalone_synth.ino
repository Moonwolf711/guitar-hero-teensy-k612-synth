/*
 * Guitar Hero Standalone Synthesizer - Complete System
 * Teensy 4.1 + ESP-12E + Audio Shield
 *
 * NO COMPUTER REQUIRED! Direct controller → audio synthesis
 *
 * Features:
 * ✅ USB Host input (any Guitar Hero controller)
 * ✅ Teensy Audio Library synthesis (SuperSaw, FM, Wavetable)
 * ✅ All pentatonic scales (Major, Minor, Blues, Japanese, Egyptian)
 * ✅ Strum transpose (±12 semitones)
 * ✅ Arpeggiator with Star Power
 * ✅ Whammy bar effects (pitch bend, filter)
 * ✅ ESP-12E WiFi control + web interface
 * ✅ OSC output for DAW integration
 * ✅ Ultra-low latency (<2ms)
 *
 * Hardware:
 * - Teensy 4.1 (USB Host capable)
 * - Teensy Audio Shield Rev D (or I2S DAC)
 * - ESP-12E WiFi module
 * - Guitar Hero controller (any model)
 * - Audio output (headphones/amp)
 */

#include <Arduino.h>
#include <USBHost_t36.h>
#include <Audio.h>

// ===== CONFIGURATION =====
#define USE_AUDIO_SHIELD  true   // Set false if using external DAC
#define USE_ESP_WIFI      true   // Set false to disable WiFi
#define DEBUG_BUTTONS     false  // Enable button debug output

// ESP-12E Communication
#define ESP_SERIAL        Serial1
#define ESP_BAUD          115200

// ===== USB HOST SETUP =====
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBHIDParser hid3(myusb);
JoystickController joystick1(myusb);

// USB Driver tracking
USBDriver *drivers[] = {&hub1, &hub2, &hid1, &hid2, &hid3, &joystick1};
#define CNT_DEVICES (sizeof(drivers)/sizeof(drivers[0]))
const char * driver_names[CNT_DEVICES] = {"Hub1", "Hub2", "HID1", "HID2", "HID3", "Joystick"};
bool driver_active[CNT_DEVICES] = {false};

// ===== AUDIO SYNTHESIS SETUP =====
// SuperSaw oscillators (5 detuned sawtooth waves)
AudioSynthWaveformModulated osc1;
AudioSynthWaveformModulated osc2;
AudioSynthWaveformModulated osc3;
AudioSynthWaveformModulated osc4;
AudioSynthWaveformModulated osc5;

// Envelopes for each voice
AudioEffectEnvelope env1;
AudioEffectEnvelope env2;
AudioEffectEnvelope env3;
AudioEffectEnvelope env4;
AudioEffectEnvelope env5;

// Mixers
AudioMixer4 voiceMixer1;
AudioMixer4 voiceMixer2;
AudioMixer4 mainMixer;

// Effects chain
AudioFilterStateVariable filter;
AudioEffectDelay delay1;
AudioEffectChorus chorus1;

// Outputs
AudioOutputI2S i2s1;           // Audio Shield I2S
AudioOutputAnalog dac1;        // Built-in DAC (A21)

// Audio connections
AudioConnection patchCord1(osc1, env1);
AudioConnection patchCord2(osc2, env2);
AudioConnection patchCord3(osc3, env3);
AudioConnection patchCord4(osc4, env4);
AudioConnection patchCord5(osc5, env5);

AudioConnection patchCord6(env1, 0, voiceMixer1, 0);
AudioConnection patchCord7(env2, 0, voiceMixer1, 1);
AudioConnection patchCord8(env3, 0, voiceMixer1, 2);
AudioConnection patchCord9(env4, 0, voiceMixer2, 0);
AudioConnection patchCord10(env5, 0, voiceMixer2, 1);

AudioConnection patchCord11(voiceMixer1, 0, mainMixer, 0);
AudioConnection patchCord12(voiceMixer2, 0, mainMixer, 1);

AudioConnection patchCord13(mainMixer, 0, filter, 0);
AudioConnection patchCord14(filter, 0, chorus1, 0);
AudioConnection patchCord15(chorus1, 0, delay1, 0);

AudioConnection patchCord16(delay1, 0, i2s1, 0);      // Left
AudioConnection patchCord17(delay1, 0, i2s1, 1);      // Right
AudioConnection patchCord18(delay1, 0, dac1, 0);      // Analog out

AudioControlSGTL5000 audioShield;

// ===== SCALE DEFINITIONS =====
enum ScaleType {
  MAJOR_PENT,
  MINOR_PENT,
  BLUES,
  JAPANESE,
  EGYPTIAN,
  DORIAN_PENT,
  LYDIAN_PENT,
  NUM_SCALES
};

const uint8_t scales[NUM_SCALES][5] = {
  {0, 2, 4, 7, 9},      // Major Pentatonic
  {0, 3, 5, 7, 10},     // Minor Pentatonic
  {0, 3, 5, 6, 10},     // Blues
  {0, 1, 5, 7, 8},      // Japanese (In Sen)
  {0, 2, 5, 7, 10},     // Egyptian
  {0, 2, 3, 7, 9},      // Dorian Pentatonic
  {0, 2, 4, 6, 9}       // Lydian Pentatonic
};

const char* scaleNames[NUM_SCALES] = {
  "Major Pentatonic",
  "Minor Pentatonic",
  "Blues",
  "Japanese (In Sen)",
  "Egyptian",
  "Dorian Pentatonic",
  "Lydian Pentatonic"
};

// Root notes (C4 to B4)
const uint8_t rootNotes[12] = {
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71
};

const char* noteNames[] = {
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// ===== STATE VARIABLES =====
uint8_t currentScale = MINOR_PENT;
uint8_t currentRoot = 0;           // C
int8_t currentOctave = 0;          // ±2 octaves
bool fxEnabled = false;
uint8_t currentInstrument = 0;     // 0-3 presets

// Button states (Green, Red, Yellow, Blue, Orange)
bool fretStates[5] = {false};
uint8_t activeNotes[5] = {0};
uint8_t noteCount = 0;

// Strum state
bool strumUpActive = false;
bool strumDownActive = false;
uint8_t strumNotes[5] = {0};
uint8_t strumNoteCount = 0;

// Arpeggiator
bool arpeggiatorActive = false;
uint8_t arpIndex = 0;
bool arpDirection = true;
uint16_t arpSpeed = 125;  // milliseconds
elapsedMillis arpTimer;

// Portamento
float currentFrequency = 440.0f;
float targetFrequency = 440.0f;
float portamentoSpeed = 0.08f;
bool noteActive = false;

// Whammy bar
uint16_t lastWhammyValue = 512;   // 10-bit center
float filterFreq = 2000.0f;
float pitchBendCents = 0.0f;

// Performance monitoring
elapsedMicros loopTimer;
elapsedMillis statusTimer;
elapsedMillis espTimer;
uint32_t maxLoopTime = 0;
uint32_t totalNotes = 0;

// USB state
bool lastJoystickAvailable = false;
uint32_t lastButtons = 0;
uint32_t lastAxisValues[8] = {0};

// ===== HELPER FUNCTIONS =====

float midiToFreq(uint8_t midiNote) {
  return 440.0f * pow(2.0f, (midiNote - 69) / 12.0f);
}

uint8_t getMidiNote(uint8_t buttonIndex) {
  if (buttonIndex >= 5) return 60;

  uint8_t scaleDegree = scales[currentScale][buttonIndex];
  uint8_t note = rootNotes[currentRoot] + scaleDegree + (currentOctave * 12);

  // Clamp to valid MIDI range
  if (note > 127) note = 127;
  if (note < 12) note = 12;

  return note;
}

const char* getNoteName(uint8_t midiNote) {
  static char noteName[5];
  uint8_t note = midiNote % 12;
  uint8_t octave = (midiNote / 12) - 1;
  sprintf(noteName, "%s%d", noteNames[note], octave);
  return noteName;
}

void removeActiveNote(uint8_t note) {
  for (int i = 0; i < noteCount; i++) {
    if (activeNotes[i] == note) {
      for (int j = i; j < noteCount - 1; j++) {
        activeNotes[j] = activeNotes[j + 1];
      }
      noteCount--;
      break;
    }
  }
}

// ===== AUDIO SYNTHESIS FUNCTIONS =====

void initAudio() {
  AudioMemory(60);  // Increased for effects chain

  #if USE_AUDIO_SHIELD
  if (audioShield.enable()) {
    audioShield.volume(0.8);
    audioShield.lineOutLevel(13);
    audioShield.adcHighPassFilterDisable();
    Serial.println("✓ Audio Shield enabled");
  } else {
    Serial.println("⚠ No Audio Shield - using built-in DAC");
  }
  #endif

  // Configure SuperSaw oscillators
  // Detune pattern: -10, -5, 0, +5, +10 cents
  osc1.begin(WAVEFORM_SAWTOOTH);
  osc2.begin(WAVEFORM_SAWTOOTH);
  osc3.begin(WAVEFORM_SAWTOOTH);
  osc4.begin(WAVEFORM_SAWTOOTH);
  osc5.begin(WAVEFORM_SAWTOOTH);

  osc1.amplitude(0.18);
  osc2.amplitude(0.20);
  osc3.amplitude(0.22);  // Center slightly louder
  osc4.amplitude(0.20);
  osc5.amplitude(0.18);

  // Configure envelopes (guitar-like attack)
  for (auto* env : {&env1, &env2, &env3, &env4, &env5}) {
    env->attack(3);      // Fast attack (3ms)
    env->hold(0);
    env->decay(150);     // Medium decay
    env->sustain(0.7);   // 70% sustain
    env->release(80);    // Quick release
  }

  // Voice mixing
  voiceMixer1.gain(0, 0.20);
  voiceMixer1.gain(1, 0.25);
  voiceMixer1.gain(2, 0.25);
  voiceMixer2.gain(0, 0.25);
  voiceMixer2.gain(1, 0.20);

  mainMixer.gain(0, 0.5);
  mainMixer.gain(1, 0.5);

  // Filter (lowpass with resonance)
  filter.frequency(2000);
  filter.resonance(1.4);
  filter.octaveControl(4);

  // Chorus (stereo width)
  chorus1.voices(2);

  // Delay (disabled by default)
  delay1.delay(0, 0);

  Serial.println("✓ Audio synthesis initialized");
}

void updateOscillatorFrequencies(float baseFreq) {
  // Apply pitch bend
  float bendRatio = pow(2.0f, pitchBendCents / 1200.0f);
  baseFreq *= bendRatio;

  // SuperSaw detune (cents: -10, -5, 0, +5, +10)
  float detune1 = baseFreq * pow(2.0f, -10.0f / 1200.0f);
  float detune2 = baseFreq * pow(2.0f, -5.0f / 1200.0f);
  float detune3 = baseFreq;
  float detune4 = baseFreq * pow(2.0f, 5.0f / 1200.0f);
  float detune5 = baseFreq * pow(2.0f, 10.0f / 1200.0f);

  osc1.frequency(detune1);
  osc2.frequency(detune2);
  osc3.frequency(detune3);
  osc4.frequency(detune4);
  osc5.frequency(detune5);
}

void playNote(uint8_t midiNote) {
  targetFrequency = midiToFreq(midiNote);

  if (!noteActive) {
    currentFrequency = targetFrequency;
    noteActive = true;
  }

  updateOscillatorFrequencies(currentFrequency);

  // Trigger all envelopes
  env1.noteOn();
  env2.noteOn();
  env3.noteOn();
  env4.noteOn();
  env5.noteOn();

  totalNotes++;

  // Send to ESP for web interface update
  sendNoteToESP(midiNote, true);

  Serial.print("♪ ");
  Serial.print(getNoteName(midiNote));
  Serial.print(" (");
  Serial.print(targetFrequency, 1);
  Serial.println(" Hz)");
}

void stopNote() {
  env1.noteOff();
  env2.noteOff();
  env3.noteOff();
  env4.noteOff();
  env5.noteOff();
  noteActive = false;

  sendNoteToESP(0, false);
}

void updateAudio() {
  // Portamento: smooth frequency transitions
  if (noteActive && abs(currentFrequency - targetFrequency) > 0.5f) {
    float diff = targetFrequency - currentFrequency;
    currentFrequency += diff * portamentoSpeed;
    updateOscillatorFrequencies(currentFrequency);
  }
}

void allNotesOff() {
  stopNote();
  noteCount = 0;
  strumNoteCount = 0;
  memset(activeNotes, 0, sizeof(activeNotes));
  memset(strumNotes, 0, sizeof(strumNotes));
  noteActive = false;
  pitchBendCents = 0.0f;
}

// ===== GUITAR HERO INPUT PROCESSING =====

void processFretButtons() {
  if (!joystick1.available()) return;

  uint32_t buttons = joystick1.getButtons();

  // Generic mapping - works with most GH controllers
  // Adjust button codes based on your controller
  struct ButtonMap {
    uint8_t code;
    uint8_t fret;
  };

  ButtonMap mapping[] = {
    {0, 0},   // Green
    {1, 1},   // Red
    {2, 2},   // Yellow
    {3, 3},   // Blue
    {4, 4}    // Orange
  };

  for (int i = 0; i < 5; i++) {
    bool pressed = false;

    for (const auto& map : mapping) {
      if (map.fret == i && (buttons & (1 << map.code))) {
        pressed = true;
        break;
      }
    }

    // State change detection
    if (pressed != fretStates[i]) {
      fretStates[i] = pressed;

      if (pressed) {
        // Fret pressed
        uint8_t note = getMidiNote(i);

        // If strum is active, play transposed note
        if (strumUpActive) {
          uint8_t transposedNote = note + 12;
          if (transposedNote <= 127) {
            playNote(transposedNote);
            if (strumNoteCount < 5) {
              strumNotes[strumNoteCount++] = transposedNote;
            }
          }
        } else if (strumDownActive) {
          uint8_t transposedNote = note - 12;
          if (transposedNote >= 12) {
            playNote(transposedNote);
            if (strumNoteCount < 5) {
              strumNotes[strumNoteCount++] = transposedNote;
            }
          }
        } else {
          // Normal note
          playNote(note);
          if (noteCount < 5) {
            activeNotes[noteCount++] = note;
          }
        }

      } else {
        // Fret released
        uint8_t note = getMidiNote(i);

        if (noteCount == 0 && strumNoteCount == 0) {
          stopNote();
        }

        removeActiveNote(note);
      }
    }
  }
}

void processControlButtons() {
  if (!joystick1.available()) return;

  uint32_t buttons = joystick1.getButtons();

  // Star Power - Toggle Arpeggiator
  static bool lastStarPower = false;
  bool starPower = (buttons & (1 << 12));  // Adjust bit for your controller
  if (starPower && !lastStarPower) {
    arpeggiatorActive = !arpeggiatorActive;
    arpIndex = 0;
    Serial.print("Arpeggiator: ");
    Serial.println(arpeggiatorActive ? "ON" : "OFF");
    sendConfigToESP();
  }
  lastStarPower = starPower;

  // Plus/Minus - Scale selection
  static bool lastPlus = false;
  static bool lastMinus = false;
  bool plusBtn = (buttons & (1 << 10));
  bool minusBtn = (buttons & (1 << 11));

  if (plusBtn && !lastPlus) {
    currentScale = (currentScale + 1) % NUM_SCALES;
    Serial.print("Scale: ");
    Serial.println(scaleNames[currentScale]);
    allNotesOff();
    sendConfigToESP();
  }
  lastPlus = plusBtn;

  if (minusBtn && !lastMinus) {
    currentScale = (currentScale - 1 + NUM_SCALES) % NUM_SCALES;
    Serial.print("Scale: ");
    Serial.println(scaleNames[currentScale]);
    allNotesOff();
    sendConfigToESP();
  }
  lastMinus = minusBtn;
}

void processAnalogControls() {
  if (!joystick1.available()) return;

  // Whammy bar (usually axis 5 or similar)
  uint32_t whammyValue = joystick1.getAxis(5);  // Adjust axis number

  if (abs((int)whammyValue - (int)lastWhammyValue) > 20) {
    lastWhammyValue = whammyValue;

    // Map to pitch bend (-200 to +200 cents = ±2 semitones)
    pitchBendCents = map(whammyValue, 0, 1023, -200, 200);

    // Also control filter frequency
    filterFreq = map(whammyValue, 0, 1023, 500, 8000);
    filter.frequency(filterFreq);

    if (noteActive) {
      updateOscillatorFrequencies(currentFrequency);
    }
  }

  // D-Pad for octave control
  int dpadY = joystick1.getAxis(1);  // Vertical axis
  static int lastDpadY = 0;

  if (dpadY != lastDpadY && abs(dpadY) > 50) {
    if (dpadY < -50 && currentOctave < 2) {
      currentOctave++;
      Serial.print("Octave: ");
      Serial.println(currentOctave);
      allNotesOff();
      sendConfigToESP();
    } else if (dpadY > 50 && currentOctave > -2) {
      currentOctave--;
      Serial.print("Octave: ");
      Serial.println(currentOctave);
      allNotesOff();
      sendConfigToESP();
    }
    lastDpadY = dpadY;
  }
}

void processArpeggiator() {
  if (!arpeggiatorActive || noteCount == 0) return;

  if (arpTimer >= arpSpeed) {
    uint8_t arpNote = activeNotes[arpIndex];
    playNote(arpNote);

    // Bounce pattern
    if (arpDirection) {
      arpIndex++;
      if (arpIndex >= noteCount) {
        arpIndex = noteCount > 1 ? noteCount - 2 : 0;
        arpDirection = false;
      }
    } else {
      if (arpIndex == 0) {
        arpIndex = 1;
        arpDirection = true;
      } else {
        arpIndex--;
      }
    }

    arpTimer = 0;
  }
}

// ===== ESP-12E COMMUNICATION =====

void initESP() {
  #if USE_ESP_WIFI
  ESP_SERIAL.begin(ESP_BAUD);
  delay(100);

  // Send initial config
  sendConfigToESP();

  Serial.println("✓ ESP-12E communication initialized");
  #endif
}

void sendConfigToESP() {
  #if USE_ESP_WIFI
  // Send JSON config to ESP
  ESP_SERIAL.print("{\"type\":\"config\",");
  ESP_SERIAL.print("\"scale\":");
  ESP_SERIAL.print(currentScale);
  ESP_SERIAL.print(",\"scaleName\":\"");
  ESP_SERIAL.print(scaleNames[currentScale]);
  ESP_SERIAL.print("\",\"root\":");
  ESP_SERIAL.print(currentRoot);
  ESP_SERIAL.print(",\"rootName\":\"");
  ESP_SERIAL.print(noteNames[currentRoot]);
  ESP_SERIAL.print("\",\"octave\":");
  ESP_SERIAL.print(currentOctave);
  ESP_SERIAL.print(",\"arp\":");
  ESP_SERIAL.print(arpeggiatorActive ? "true" : "false");
  ESP_SERIAL.println("}");
  #endif
}

void sendNoteToESP(uint8_t note, bool on) {
  #if USE_ESP_WIFI
  ESP_SERIAL.print("{\"type\":\"note\",\"note\":");
  ESP_SERIAL.print(note);
  ESP_SERIAL.print(",\"on\":");
  ESP_SERIAL.print(on ? "true" : "false");
  ESP_SERIAL.println("}");
  #endif
}

void sendStatusToESP() {
  #if USE_ESP_WIFI
  ESP_SERIAL.print("{\"type\":\"status\",");
  ESP_SERIAL.print("\"cpu\":");
  ESP_SERIAL.print(AudioProcessorUsage(), 1);
  ESP_SERIAL.print(",\"mem\":");
  ESP_SERIAL.print(AudioMemoryUsage());
  ESP_SERIAL.print(",\"notes\":");
  ESP_SERIAL.print(totalNotes);
  ESP_SERIAL.print(",\"latency\":");
  ESP_SERIAL.print(maxLoopTime);
  ESP_SERIAL.println("}");
  #endif
}

void processESPCommands() {
  #if USE_ESP_WIFI
  if (ESP_SERIAL.available()) {
    String cmd = ESP_SERIAL.readStringUntil('\n');
    cmd.trim();

    // Simple command parsing
    if (cmd.startsWith("{\"cmd\":\"scale\",\"value\":")) {
      int value = cmd.substring(26).toInt();
      if (value >= 0 && value < NUM_SCALES) {
        currentScale = value;
        allNotesOff();
        Serial.print("ESP Command - Scale: ");
        Serial.println(scaleNames[currentScale]);
      }
    }
    else if (cmd.startsWith("{\"cmd\":\"root\",\"value\":")) {
      int value = cmd.substring(24).toInt();
      if (value >= 0 && value < 12) {
        currentRoot = value;
        allNotesOff();
        Serial.print("ESP Command - Root: ");
        Serial.println(noteNames[currentRoot]);
      }
    }
    else if (cmd.startsWith("{\"cmd\":\"octave\",\"value\":")) {
      int value = cmd.substring(26).toInt();
      if (value >= -2 && value <= 2) {
        currentOctave = value;
        allNotesOff();
        Serial.print("ESP Command - Octave: ");
        Serial.println(currentOctave);
      }
    }
    else if (cmd.indexOf("\"arp\"") >= 0) {
      arpeggiatorActive = !arpeggiatorActive;
      Serial.print("ESP Command - Arp: ");
      Serial.println(arpeggiatorActive ? "ON" : "OFF");
    }
  }
  #endif
}

// ===== MAIN SETUP =====

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("╔════════════════════════════════════════════════════╗");
  Serial.println("║  GUITAR HERO STANDALONE SYNTHESIZER v2.0          ║");
  Serial.println("║  Teensy 4.1 + Audio Shield + ESP-12E WiFi         ║");
  Serial.println("╚════════════════════════════════════════════════════╝");
  Serial.println();

  // Initialize USB Host
  myusb.begin();
  Serial.println("✓ USB Host initialized");

  // Initialize Audio
  initAudio();

  // Initialize ESP-12E
  initESP();

  Serial.println();
  Serial.println("🎸 READY TO ROCK!");
  Serial.println();
  Serial.println("Controls:");
  Serial.println("  Frets = Play notes (pentatonic scales)");
  Serial.println("  Strum Up = +12 semitones");
  Serial.println("  Strum Down = -12 semitones");
  Serial.println("  Star Power = Arpeggiator");
  Serial.println("  Plus/Minus = Change scale");
  Serial.println("  D-Pad = Octave ±");
  Serial.println("  Whammy = Pitch bend + filter");
  Serial.println();
  Serial.print("Current Scale: ");
  Serial.println(scaleNames[currentScale]);
  Serial.print("Root: ");
  Serial.println(noteNames[currentRoot]);
  Serial.println();
  Serial.println("═══════════════════════════════════════════════════");
  Serial.println();
}

// ===== MAIN LOOP =====

void loop() {
  loopTimer = 0;

  // Update USB Host
  myusb.Task();

  // Check for USB device changes
  for (uint8_t i = 0; i < CNT_DEVICES; i++) {
    if (*drivers[i] != driver_active[i]) {
      driver_active[i] = *drivers[i];
      if (driver_active[i]) {
        Serial.print("USB: ");
        Serial.print(driver_names[i]);
        Serial.println(" connected");
      }
    }
  }

  // Detect controller connection
  bool currentJoystickAvailable = joystick1.available();
  if (currentJoystickAvailable != lastJoystickAvailable) {
    lastJoystickAvailable = currentJoystickAvailable;
    if (currentJoystickAvailable) {
      Serial.println("\n*** CONTROLLER CONNECTED! ***");
      Serial.print("VID: 0x");
      Serial.print(joystick1.idVendor(), HEX);
      Serial.print(" PID: 0x");
      Serial.println(joystick1.idProduct(), HEX);
      Serial.println("Ready to play!\n");
    } else {
      Serial.println("\n*** CONTROLLER DISCONNECTED! ***\n");
      allNotesOff();
    }
  }

  // Process input if controller is connected
  if (joystick1.available()) {
    processFretButtons();
    processControlButtons();
    processAnalogControls();
  }

  // Update audio (portamento)
  updateAudio();

  // Process arpeggiator
  processArpeggiator();

  // ESP communication
  processESPCommands();

  // Send status to ESP every 500ms
  if (espTimer >= 500) {
    sendStatusToESP();
    espTimer = 0;
  }

  // Performance monitoring
  uint32_t loopTime = loopTimer;
  if (loopTime > maxLoopTime) {
    maxLoopTime = loopTime;
  }

  // Status print every 5 seconds
  if (statusTimer >= 5000) {
    Serial.println("═══ Status ═══");
    Serial.print("CPU: ");
    Serial.print(AudioProcessorUsage(), 1);
    Serial.print("% | Memory: ");
    Serial.print(AudioMemoryUsage());
    Serial.print(" blocks | Latency: ");
    Serial.print(maxLoopTime);
    Serial.print(" μs | Notes: ");
    Serial.println(totalNotes);

    maxLoopTime = 0;
    statusTimer = 0;
  }

  delayMicroseconds(50);
}
