/**
 * Guitar Hero Controller Synthesizer for Teensy 4.1
 * Main Firmware Implementation
 *
 * Hardware:
 * - Teensy 4.1 (ARM Cortex-M7 @ 600MHz)
 * - USB Host for Xbox 360 Guitar Hero Controller
 * - Audio output via I2S (PCM5102A DAC recommended)
 * - ESP8266 for WiFi control (Serial1)
 *
 * Audio Specifications:
 * - Sample Rate: 44.1kHz
 * - Bit Depth: 16-bit
 * - Polyphony: 6 voices
 * - Target Latency: <5ms
 */

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <USBHost_t36.h>
#include <MIDI.h>

#include "gh_controller.h"
#include "synth_engine.h"
#include "scale_quantizer.h"
#include "config.h"

// USB Host objects
USBHost myusb;
USBHub hub1(myusb);
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBHIDParser hid3(myusb);

// Custom Guitar Hero controller driver
GuitarHeroController ghController(myusb);

// Audio system objects - 6 voice polyphonic synthesizer
// Using PCM5102A DAC for better quality and simpler wiring (no control lines needed)
AudioSynthWaveformModulated voice1;
AudioSynthWaveformModulated voice2;
AudioSynthWaveformModulated voice3;
AudioSynthWaveformModulated voice4;
AudioSynthWaveformModulated voice5;
AudioSynthWaveformModulated voice6;

AudioEffectEnvelope env1;
AudioEffectEnvelope env2;
AudioEffectEnvelope env3;
AudioEffectEnvelope env4;
AudioEffectEnvelope env5;
AudioEffectEnvelope env6;

AudioFilterStateVariable filter1;
AudioFilterStateVariable filter2;
AudioFilterStateVariable filter3;
AudioFilterStateVariable filter4;
AudioFilterStateVariable filter5;
AudioFilterStateVariable filter6;

AudioMixer4 voiceMixer1;  // Voices 1-4
AudioMixer4 voiceMixer2;  // Voices 5-6 + effects return
AudioMixer4 mainMixer;    // Final mix

AudioEffectReverb reverb;
AudioEffectDelay delay1;
AudioMixer4 effectsSend;
AudioMixer4 effectsReturn;

AudioOutputI2S i2s_out;
AudioConnection patchCords[30];  // We'll initialize these in setup()

// Synthesizer engine
SynthEngine synthEngine;
ScaleQuantizer scaleQuantizer;

// Performance monitoring
elapsedMillis perfTimer;
uint32_t loopCount = 0;
float cpuUsageMax = 0;
float memoryUsageMax = 0;

// Serial communication with ESP8266
HardwareSerial &ESP_SERIAL = Serial1;  // TX1(pin 1), RX1(pin 0)
const uint32_t ESP_BAUD = 115200;

// State variables
bool controllerConnected = false;
uint32_t lastControllerUpdate = 0;
uint8_t currentScale = 0;  // 0-5 for 6 scales
int8_t octaveShift = 0;    // -2 to +2 octaves
float pitchBend = 0.0f;    // -1.0 to +1.0 (normalized from whammy bar)

// Voice allocation
struct Voice {
    bool active;
    uint8_t note;
    uint8_t velocity;
    uint32_t startTime;
    AudioSynthWaveformModulated* waveform;
    AudioEffectEnvelope* envelope;
    AudioFilterStateVariable* filter;
};

Voice voices[NUM_VOICES];

// Function prototypes
void setupAudio();
void setupUSBHost();
void processControllerInput();
void updateSynthParameters();
void noteOn(uint8_t note, uint8_t velocity);
void noteOff(uint8_t note);
int8_t allocateVoice();
void releaseVoice(uint8_t voiceIndex);
void sendESPStatus();
void handleSerialCommand();
void performanceReport();

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    while (!Serial && millis() < 3000);  // Wait up to 3 seconds for serial

    Serial.println(F("Guitar Hero Synthesizer v1.0"));
    Serial.println(F("Initializing..."));

    // Initialize ESP8266 serial
    ESP_SERIAL.begin(ESP_BAUD);

    // Initialize audio system
    AudioMemory(64);  // Allocate audio memory blocks
    setupAudio();

    // Initialize USB Host
    Serial.println(F("Starting USB Host..."));
    myusb.begin();

    // Initialize synthesizer engine
    synthEngine.init();

    // Initialize scale quantizer with Pentatonic Minor as default
    scaleQuantizer.setScale(SCALE_PENTATONIC_MINOR);

    // Initialize voice structures
    voices[0] = {false, 0, 0, 0, &voice1, &env1, &filter1};
    voices[1] = {false, 0, 0, 0, &voice2, &env2, &filter2};
    voices[2] = {false, 0, 0, 0, &voice3, &env3, &filter3};
    voices[3] = {false, 0, 0, 0, &voice4, &env4, &filter4};
    voices[4] = {false, 0, 0, 0, &voice5, &env5, &filter5};
    voices[5] = {false, 0, 0, 0, &voice6, &env6, &filter6};

    // Configure waveforms - start with sawtooth for rich harmonics
    for (int i = 0; i < NUM_VOICES; i++) {
        voices[i].waveform->begin(WAVEFORM_SAWTOOTH);
        voices[i].waveform->amplitude(0.8);
        voices[i].waveform->frequency(440.0);
        voices[i].waveform->frequencyModulation(1.0);

        // Configure ADSR envelope
        voices[i].envelope->attack(5.0);
        voices[i].envelope->hold(0.0);
        voices[i].envelope->decay(50.0);
        voices[i].envelope->sustain(0.7);
        voices[i].envelope->release(300.0);

        // Configure filter - low pass with moderate resonance
        voices[i].filter->frequency(2000.0);
        voices[i].filter->resonance(2.0);
        voices[i].filter->octaveControl(1.0);
    }

    // Configure effects
    reverb.roomsize(0.7);
    delay1.delay(0, 150.0);  // 150ms delay

    // Set initial mixer levels
    voiceMixer1.gain(0, 0.25);  // Voice 1
    voiceMixer1.gain(1, 0.25);  // Voice 2
    voiceMixer1.gain(2, 0.25);  // Voice 3
    voiceMixer1.gain(3, 0.25);  // Voice 4

    voiceMixer2.gain(0, 0.25);  // Voice 5
    voiceMixer2.gain(1, 0.25);  // Voice 6
    voiceMixer2.gain(2, 0.5);   // Effects return

    mainMixer.gain(0, 0.5);     // Voice mixer 1
    mainMixer.gain(1, 0.5);     // Voice mixer 2
    mainMixer.gain(2, 0.0);     // Unused
    mainMixer.gain(3, 0.0);     // Unused

    effectsSend.gain(0, 0.3);   // Reverb send
    effectsSend.gain(1, 0.2);   // Delay send

    effectsReturn.gain(0, 0.5); // Reverb return
    effectsReturn.gain(1, 0.5); // Delay return

    Serial.println(F("Initialization complete!"));
    Serial.println(F("Waiting for Guitar Hero controller..."));
}

void loop() {
    // Process USB Host tasks
    myusb.Task();

    // Check if controller is connected
    if (ghController.connected()) {
        if (!controllerConnected) {
            controllerConnected = true;
            Serial.println(F("Guitar Hero controller connected!"));
            sendESPStatus();
        }

        // Process controller input
        processControllerInput();

    } else {
        if (controllerConnected) {
            controllerConnected = false;
            Serial.println(F("Guitar Hero controller disconnected!"));
            // Release all notes
            for (int i = 0; i < NUM_VOICES; i++) {
                if (voices[i].active) {
                    releaseVoice(i);
                }
            }
            sendESPStatus();
        }
    }

    // Update synthesizer parameters based on controls
    updateSynthParameters();

    // Handle ESP8266 serial communication
    if (ESP_SERIAL.available()) {
        handleSerialCommand();
    }

    // Performance monitoring (every second)
    if (perfTimer >= 1000) {
        performanceReport();
        perfTimer = 0;
        loopCount = 0;
    }
    loopCount++;

    // Keep loop fast - target >1000Hz for low latency
    delayMicroseconds(100);
}

void setupAudio() {
    Serial.println(F("Configuring audio system..."));

    // Create audio connections
    // Voice 1 path
    patchCords[0] = AudioConnection(voice1, env1);
    patchCords[1] = AudioConnection(env1, 0, filter1, 0);
    patchCords[2] = AudioConnection(filter1, 0, voiceMixer1, 0);

    // Voice 2 path
    patchCords[3] = AudioConnection(voice2, env2);
    patchCords[4] = AudioConnection(env2, 0, filter2, 0);
    patchCords[5] = AudioConnection(filter2, 0, voiceMixer1, 1);

    // Voice 3 path
    patchCords[6] = AudioConnection(voice3, env3);
    patchCords[7] = AudioConnection(env3, 0, filter3, 0);
    patchCords[8] = AudioConnection(filter3, 0, voiceMixer1, 2);

    // Voice 4 path
    patchCords[9] = AudioConnection(voice4, env4);
    patchCords[10] = AudioConnection(env4, 0, filter4, 0);
    patchCords[11] = AudioConnection(filter4, 0, voiceMixer1, 3);

    // Voice 5 path
    patchCords[12] = AudioConnection(voice5, env5);
    patchCords[13] = AudioConnection(env5, 0, filter5, 0);
    patchCords[14] = AudioConnection(filter5, 0, voiceMixer2, 0);

    // Voice 6 path
    patchCords[15] = AudioConnection(voice6, env6);
    patchCords[16] = AudioConnection(env6, 0, filter6, 0);
    patchCords[17] = AudioConnection(filter6, 0, voiceMixer2, 1);

    // Effects sends
    patchCords[18] = AudioConnection(voiceMixer1, 0, effectsSend, 0);
    patchCords[19] = AudioConnection(voiceMixer2, 0, effectsSend, 1);

    // Effects processing
    patchCords[20] = AudioConnection(effectsSend, reverb);
    patchCords[21] = AudioConnection(effectsSend, delay1);
    patchCords[22] = AudioConnection(reverb, 0, effectsReturn, 0);
    patchCords[23] = AudioConnection(delay1, 0, effectsReturn, 1);

    // Effects return to voice mixer 2
    patchCords[24] = AudioConnection(effectsReturn, 0, voiceMixer2, 2);

    // Main mix
    patchCords[25] = AudioConnection(voiceMixer1, 0, mainMixer, 0);
    patchCords[26] = AudioConnection(voiceMixer2, 0, mainMixer, 1);

    // Output to I2S
    patchCords[27] = AudioConnection(mainMixer, 0, i2s_out, 0);
    patchCords[28] = AudioConnection(mainMixer, 0, i2s_out, 1);

    Serial.println(F("Audio system configured"));
}

void processControllerInput() {
    // Get controller state
    GHControllerState state = ghController.getState();

    static GHControllerState lastState;
    static bool fretStates[5] = {false, false, false, false, false};

    // Process fret buttons (Green, Red, Yellow, Blue, Orange)
    bool newFretStates[5] = {
        state.greenFret,
        state.redFret,
        state.yellowFret,
        state.blueFret,
        state.orangeFret
    };

    // Check for scale selection (Minus button + fret)
    if (state.minusButton) {
        for (int i = 0; i < 5; i++) {
            if (newFretStates[i] && !fretStates[i]) {
                // Change scale
                currentScale = i;
                scaleQuantizer.setScale(currentScale);
                Serial.print(F("Scale changed to: "));
                Serial.println(scaleQuantizer.getScaleName(currentScale));
                sendESPStatus();
            }
        }
    } else {
        // Normal note triggering
        for (int i = 0; i < 5; i++) {
            if (newFretStates[i] != fretStates[i]) {
                if (newFretStates[i]) {
                    // Note on - map fret to scale degree
                    uint8_t scaleDegree = i;
                    uint8_t midiNote = scaleQuantizer.quantizeNote(scaleDegree, octaveShift);

                    // Apply pitch bend from whammy bar
                    if (state.whammyBar > 10) {  // Deadzone
                        float bendAmount = (state.whammyBar - 10) / 245.0f;  // Normalize to 0-1
                        pitchBend = bendAmount * 2.0f;  // +2 semitones max
                    } else {
                        pitchBend = 0.0f;
                    }

                    noteOn(midiNote, 100);  // Fixed velocity for now
                } else {
                    // Note off
                    uint8_t scaleDegree = i;
                    uint8_t midiNote = scaleQuantizer.quantizeNote(scaleDegree, octaveShift);
                    noteOff(midiNote);
                }
                fretStates[i] = newFretStates[i];
            }
        }
    }

    // Star Power button - octave boost
    if (state.starPower != lastState.starPower) {
        if (state.starPower) {
            octaveShift = 1;  // +1 octave
            Serial.println(F("Star Power: Octave UP"));
        } else {
            octaveShift = 0;  // Normal octave
            Serial.println(F("Star Power: Normal octave"));
        }
    }

    // Pickup selector - tone presets
    static uint8_t lastPickup = 0;
    if (state.pickupSelector != lastPickup) {
        switch (state.pickupSelector) {
            case 0:  // Bridge pickup - bright
                synthEngine.setTonePreset(TONE_BRIGHT);
                break;
            case 1:  // Middle - balanced
                synthEngine.setTonePreset(TONE_BALANCED);
                break;
            case 2:  // Neck - warm
                synthEngine.setTonePreset(TONE_WARM);
                break;
        }
        lastPickup = state.pickupSelector;
        Serial.print(F("Tone preset: "));
        Serial.println(state.pickupSelector);
    }

    // D-pad and transport controls
    if (state.plusButton != lastState.plusButton && state.plusButton) {
        // Transport play/stop
        Serial.println(F("Transport: Play/Stop"));
        sendESPStatus();
    }

    // Update tilt for additional expression
    if (abs(state.tiltX - lastState.tiltX) > 5) {
        // Use tilt for filter cutoff modulation
        float tiltNorm = (state.tiltX + 32768) / 65536.0f;  // Normalize to 0-1
        float filterFreq = 500.0f + (tiltNorm * 3500.0f);   // 500Hz to 4000Hz

        for (int i = 0; i < NUM_VOICES; i++) {
            voices[i].filter->frequency(filterFreq);
        }
    }

    lastState = state;
    lastControllerUpdate = millis();
}

void noteOn(uint8_t note, uint8_t velocity) {
    // Allocate a voice for this note
    int8_t voiceIndex = allocateVoice();
    if (voiceIndex < 0) {
        Serial.println(F("No free voices!"));
        return;
    }

    Voice& voice = voices[voiceIndex];
    voice.active = true;
    voice.note = note;
    voice.velocity = velocity;
    voice.startTime = millis();

    // Calculate frequency with pitch bend
    float frequency = 440.0f * powf(2.0f, (note - 69 + pitchBend) / 12.0f);

    // Set voice parameters
    voice.waveform->frequency(frequency);
    voice.waveform->amplitude(velocity / 127.0f * 0.8f);

    // Trigger envelope
    voice.envelope->noteOn();

    Serial.print(F("Note ON: "));
    Serial.print(note);
    Serial.print(F(" Voice: "));
    Serial.print(voiceIndex);
    Serial.print(F(" Freq: "));
    Serial.println(frequency);
}

void noteOff(uint8_t note) {
    // Find the voice playing this note
    for (int i = 0; i < NUM_VOICES; i++) {
        if (voices[i].active && voices[i].note == note) {
            releaseVoice(i);
            Serial.print(F("Note OFF: "));
            Serial.print(note);
            Serial.print(F(" Voice: "));
            Serial.println(i);
            break;
        }
    }
}

int8_t allocateVoice() {
    // First, look for a free voice
    for (int i = 0; i < NUM_VOICES; i++) {
        if (!voices[i].active) {
            return i;
        }
    }

    // Voice stealing: find the oldest voice
    uint32_t oldestTime = 0xFFFFFFFF;
    int8_t oldestVoice = -1;

    for (int i = 0; i < NUM_VOICES; i++) {
        if (voices[i].startTime < oldestTime) {
            oldestTime = voices[i].startTime;
            oldestVoice = i;
        }
    }

    if (oldestVoice >= 0) {
        releaseVoice(oldestVoice);
    }

    return oldestVoice;
}

void releaseVoice(uint8_t voiceIndex) {
    if (voiceIndex >= NUM_VOICES) return;

    Voice& voice = voices[voiceIndex];
    voice.envelope->noteOff();
    voice.active = false;
    voice.note = 0;
}

void updateSynthParameters() {
    // This function updates global synth parameters based on current control states
    // Called every loop iteration for smooth parameter changes

    // Apply any LFO or modulation here
    static elapsedMillis lfoTimer;
    static float lfoPhase = 0;

    if (lfoTimer >= 10) {  // 100Hz LFO update rate
        lfoTimer = 0;
        lfoPhase += 0.0628f;  // 1Hz LFO
        if (lfoPhase > 6.283f) lfoPhase -= 6.283f;

        // Subtle vibrato when whammy is engaged
        if (pitchBend > 0) {
            float vibrato = sinf(lfoPhase * 5.0f) * 0.05f * pitchBend;
            for (int i = 0; i < NUM_VOICES; i++) {
                if (voices[i].active) {
                    float baseFreq = 440.0f * powf(2.0f, (voices[i].note - 69) / 12.0f);
                    voices[i].waveform->frequency(baseFreq * (1.0f + vibrato));
                }
            }
        }
    }
}

void sendESPStatus() {
    // Send status update to ESP8266 as JSON
    ESP_SERIAL.print(F("{\"connected\":"));
    ESP_SERIAL.print(controllerConnected ? F("true") : F("false"));
    ESP_SERIAL.print(F(",\"scale\":"));
    ESP_SERIAL.print(currentScale);
    ESP_SERIAL.print(F(",\"octave\":"));
    ESP_SERIAL.print(octaveShift);
    ESP_SERIAL.print(F(",\"cpu\":"));
    ESP_SERIAL.print(AudioProcessorUsage());
    ESP_SERIAL.print(F(",\"mem\":"));
    ESP_SERIAL.print(AudioMemoryUsage());
    ESP_SERIAL.println(F("}"));
}

void handleSerialCommand() {
    static char cmdBuffer[128];
    static uint8_t cmdIndex = 0;

    while (ESP_SERIAL.available()) {
        char c = ESP_SERIAL.read();

        if (c == '\n' || c == '\r') {
            if (cmdIndex > 0) {
                cmdBuffer[cmdIndex] = '\0';
                // Parse JSON command
                // Simple parser for basic commands
                if (strstr(cmdBuffer, "getStatus")) {
                    sendESPStatus();
                } else if (strstr(cmdBuffer, "setScale")) {
                    // Extract scale number
                    char* p = strstr(cmdBuffer, ":");
                    if (p) {
                        int scale = atoi(p + 1);
                        if (scale >= 0 && scale < 6) {
                            currentScale = scale;
                            scaleQuantizer.setScale(currentScale);
                            sendESPStatus();
                        }
                    }
                }
                cmdIndex = 0;
            }
        } else if (cmdIndex < 127) {
            cmdBuffer[cmdIndex++] = c;
        }
    }
}

void performanceReport() {
    float cpu = AudioProcessorUsage();
    float cpuMax = AudioProcessorUsageMax();
    float mem = AudioMemoryUsage();
    float memMax = AudioMemoryUsageMax();

    if (cpuMax > cpuUsageMax) cpuUsageMax = cpuMax;
    if (memMax > memoryUsageMax) memoryUsageMax = memMax;

    Serial.print(F("CPU: "));
    Serial.print(cpu);
    Serial.print(F("% (max: "));
    Serial.print(cpuMax);
    Serial.print(F("%) Memory: "));
    Serial.print(mem);
    Serial.print(F(" (max: "));
    Serial.print(memMax);
    Serial.print(F(") Loops/sec: "));
    Serial.println(loopCount);

    // Warning if CPU usage is too high
    if (cpuMax > 80.0f) {
        Serial.println(F("WARNING: CPU usage exceeding 80%!"));
    }

    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
}