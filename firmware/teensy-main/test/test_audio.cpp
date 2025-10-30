/**
 * Test Code for Audio Output
 * Generates test tones to verify audio hardware
 */

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>

// Audio objects for test tone generation
AudioSynthWaveformModulated testOsc;
AudioEffectEnvelope testEnv;
AudioOutputI2S i2s_out;

AudioConnection patchCord1(testOsc, testEnv);
AudioConnection patchCord2(testEnv, 0, i2s_out, 0);
AudioConnection patchCord3(testEnv, 0, i2s_out, 1);

// Test parameters
const float testFrequencies[] = {261.63, 329.63, 392.00, 523.25};  // C4, E4, G4, C5
const char* noteNames[] = {"C4", "E4", "G4", "C5"};
uint8_t currentNote = 0;
elapsedMillis noteTimer;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println(F("================================="));
    Serial.println(F("Audio Output Test"));
    Serial.println(F("================================="));

    // Initialize audio system
    AudioMemory(16);

    // Configure test oscillator
    testOsc.begin(WAVEFORM_SINE);
    testOsc.amplitude(0.5);
    testOsc.frequency(440.0);

    // Configure envelope
    testEnv.attack(10.0);
    testEnv.hold(0.0);
    testEnv.decay(50.0);
    testEnv.sustain(0.8);
    testEnv.release(200.0);

    Serial.println(F("Audio system initialized"));
    Serial.println(F("Playing test sequence: C4 - E4 - G4 - C5"));
    Serial.println(F("Each note plays for 1 second"));
    Serial.println();

    // PCM5102A wiring information
    Serial.println(F("PCM5102A DAC Wiring:"));
    Serial.println(F("  VCC  -> 3.3V"));
    Serial.println(F("  GND  -> GND"));
    Serial.println(F("  BCK  -> Pin 21 (BCLK)"));
    Serial.println(F("  DIN  -> Pin 7  (TX)"));
    Serial.println(F("  LCK  -> Pin 20 (LRCLK)"));
    Serial.println(F("  FMT  -> GND (I2S format)"));
    Serial.println(F("  XMT  -> 3.3V (0dB attenuation)"));
    Serial.println();

    // Trigger first note
    testOsc.frequency(testFrequencies[currentNote]);
    testEnv.noteOn();
    Serial.print(F("Playing: "));
    Serial.print(noteNames[currentNote]);
    Serial.print(F(" ("));
    Serial.print(testFrequencies[currentNote]);
    Serial.println(F(" Hz)"));
}

void loop() {
    // Play each note for 1 second
    if (noteTimer >= 1000) {
        // Release current note
        testEnv.noteOff();

        // Move to next note
        delay(200);  // Short pause between notes
        currentNote = (currentNote + 1) % 4;

        // Play next note
        testOsc.frequency(testFrequencies[currentNote]);
        testEnv.noteOn();

        Serial.print(F("Playing: "));
        Serial.print(noteNames[currentNote]);
        Serial.print(F(" ("));
        Serial.print(testFrequencies[currentNote]);
        Serial.println(F(" Hz)"));

        noteTimer = 0;
    }

    // Report audio performance
    static elapsedMillis perfTimer;
    if (perfTimer >= 5000) {
        Serial.println();
        Serial.print(F("CPU Usage: "));
        Serial.print(AudioProcessorUsage());
        Serial.print(F("% (max: "));
        Serial.print(AudioProcessorUsageMax());
        Serial.println(F("%)"));

        Serial.print(F("Memory: "));
        Serial.print(AudioMemoryUsage());
        Serial.print(F(" blocks (max: "));
        Serial.print(AudioMemoryUsageMax());
        Serial.println(F(")"));

        AudioProcessorUsageMaxReset();
        AudioMemoryUsageMaxReset();
        perfTimer = 0;
    }
}