/**
 * Scale Quantizer Module
 * Maps fret buttons to musical scales
 */

#ifndef SCALE_QUANTIZER_H
#define SCALE_QUANTIZER_H

#include <Arduino.h>

// Scale types
enum ScaleType {
    SCALE_PENTATONIC_MINOR = 0,
    SCALE_NATURAL_MINOR,
    SCALE_DORIAN,
    SCALE_HUNGARIAN_MINOR,
    SCALE_HARMONIC_MINOR,
    SCALE_PHRYGIAN,
    NUM_SCALES
};

class ScaleQuantizer {
public:
    ScaleQuantizer();

    // Set the active scale
    void setScale(uint8_t scaleIndex);

    // Set the root note (MIDI note number, default 60 = middle C)
    void setRootNote(uint8_t rootNote);

    // Quantize a scale degree (0-4 for 5 frets) to a MIDI note
    // octaveShift: -2 to +2 octaves
    uint8_t quantizeNote(uint8_t scaleDegree, int8_t octaveShift = 0);

    // Get scale name for display
    const char* getScaleName(uint8_t scaleIndex);

    // Get current scale
    uint8_t getCurrentScale() const { return currentScale; }

    // Get root note
    uint8_t getRootNote() const { return rootNote; }

private:
    uint8_t currentScale;
    uint8_t rootNote;

    // Scale intervals (semitones from root)
    // Each scale has up to 7 notes, we'll cycle through them for the 5 frets
    static const uint8_t scaleIntervals[NUM_SCALES][7];
    static const uint8_t scaleNotes[NUM_SCALES];  // Number of notes in each scale
    static const char* scaleNames[NUM_SCALES];

    // Map fret index (0-4) to scale degree
    uint8_t mapFretToScaleDegree(uint8_t fretIndex);
};

#endif // SCALE_QUANTIZER_H