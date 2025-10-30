/**
 * Test Code for Scale Quantization
 * Verifies scale mapping and note generation
 */

#include <Arduino.h>
#include "../include/scale_quantizer.h"

ScaleQuantizer quantizer;

// Test all scales with all fret combinations
void testScale(uint8_t scaleIndex) {
    quantizer.setScale(scaleIndex);

    Serial.print(F("\nTesting scale: "));
    Serial.println(quantizer.getScaleName(scaleIndex));
    Serial.println(F("Fret -> MIDI Note (Note Name)"));
    Serial.println(F("================================"));

    // Test each fret button (0-4)
    for (uint8_t fret = 0; fret < 5; fret++) {
        uint8_t midiNote = quantizer.quantizeNote(fret, 0);

        Serial.print(F("Fret "));
        Serial.print(fret);
        Serial.print(F(" -> MIDI "));
        Serial.print(midiNote);
        Serial.print(F(" ("));
        Serial.print(getNoteName(midiNote));
        Serial.println(F(")"));
    }

    // Test with octave shifts
    Serial.println(F("\nWith octave shifts:"));
    for (int8_t octave = -1; octave <= 1; octave++) {
        Serial.print(F("Octave "));
        Serial.print(octave >= 0 ? "+" : "");
        Serial.print(octave);
        Serial.print(F(": "));

        for (uint8_t fret = 0; fret < 5; fret++) {
            uint8_t midiNote = quantizer.quantizeNote(fret, octave);
            Serial.print(midiNote);
            Serial.print(F(" "));
        }
        Serial.println();
    }
}

// Convert MIDI note to note name
const char* getNoteName(uint8_t midiNote) {
    static char noteName[4];
    const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    uint8_t note = midiNote % 12;
    uint8_t octave = (midiNote / 12) - 1;

    snprintf(noteName, 4, "%s%d", notes[note], octave);
    return noteName;
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println(F("================================="));
    Serial.println(F("Scale Quantization Test"));
    Serial.println(F("================================="));

    Serial.print(F("Root note: MIDI "));
    Serial.print(quantizer.getRootNote());
    Serial.print(F(" ("));
    Serial.print(getNoteName(quantizer.getRootNote()));
    Serial.println(F(")"));

    // Test all scales
    for (uint8_t scale = 0; scale < 6; scale++) {
        testScale(scale);
        delay(1000);  // Pause between scales for readability
    }

    Serial.println(F("\n================================="));
    Serial.println(F("Scale testing complete!"));
}

void loop() {
    // Interactive testing
    if (Serial.available()) {
        char cmd = Serial.read();

        // Clear serial buffer
        while (Serial.available()) Serial.read();

        if (cmd >= '0' && cmd <= '5') {
            // Select scale
            uint8_t scale = cmd - '0';
            testScale(scale);
        } else if (cmd == 'r' || cmd == 'R') {
            // Change root note
            Serial.println(F("\nEnter new root MIDI note (0-127):"));
            while (!Serial.available());

            int newRoot = Serial.parseInt();
            if (newRoot >= 0 && newRoot <= 127) {
                quantizer.setRootNote(newRoot);
                Serial.print(F("Root note set to: "));
                Serial.print(newRoot);
                Serial.print(F(" ("));
                Serial.print(getNoteName(newRoot));
                Serial.println(F(")"));
            }
        } else if (cmd == 'h' || cmd == 'H') {
            Serial.println(F("\nCommands:"));
            Serial.println(F("0-5: Test scale by index"));
            Serial.println(F("r: Change root note"));
            Serial.println(F("h: Show this help"));
        }
    }
}