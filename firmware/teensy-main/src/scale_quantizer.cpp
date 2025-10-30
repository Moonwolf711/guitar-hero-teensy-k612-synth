/**
 * Scale Quantizer Implementation
 */

#include "scale_quantizer.h"

// Scale interval definitions (semitones from root)
const uint8_t ScaleQuantizer::scaleIntervals[NUM_SCALES][7] = {
    // Pentatonic Minor: 1, b3, 4, 5, b7
    {0, 3, 5, 7, 10, 0, 0},  // Only 5 notes

    // Natural Minor: 1, 2, b3, 4, 5, b6, b7
    {0, 2, 3, 5, 7, 8, 10},

    // Dorian: 1, 2, b3, 4, 5, 6, b7
    {0, 2, 3, 5, 7, 9, 10},

    // Hungarian Minor: 1, 2, b3, #4, 5, b6, 7
    {0, 2, 3, 6, 7, 8, 11},

    // Harmonic Minor: 1, 2, b3, 4, 5, b6, 7
    {0, 2, 3, 5, 7, 8, 11},

    // Phrygian: 1, b2, b3, 4, 5, b6, b7
    {0, 1, 3, 5, 7, 8, 10}
};

// Number of notes in each scale
const uint8_t ScaleQuantizer::scaleNotes[NUM_SCALES] = {
    5,  // Pentatonic Minor
    7,  // Natural Minor
    7,  // Dorian
    7,  // Hungarian Minor
    7,  // Harmonic Minor
    7   // Phrygian
};

// Scale names for display
const char* ScaleQuantizer::scaleNames[NUM_SCALES] = {
    "Pentatonic Minor",
    "Natural Minor",
    "Dorian",
    "Hungarian Minor",
    "Harmonic Minor",
    "Phrygian"
};

ScaleQuantizer::ScaleQuantizer() {
    currentScale = SCALE_PENTATONIC_MINOR;
    rootNote = 60;  // Middle C
}

void ScaleQuantizer::setScale(uint8_t scaleIndex) {
    if (scaleIndex < NUM_SCALES) {
        currentScale = scaleIndex;
    }
}

void ScaleQuantizer::setRootNote(uint8_t note) {
    // Clamp to valid MIDI range
    if (note <= 127) {
        rootNote = note;
    }
}

uint8_t ScaleQuantizer::quantizeNote(uint8_t scaleDegree, int8_t octaveShift) {
    // Map the 5 fret buttons to scale degrees
    uint8_t numNotes = scaleNotes[currentScale];

    // For scales with more than 5 notes, we map frets intelligently
    // For pentatonic (5 notes), it's 1:1
    // For 7-note scales, we skip certain degrees for musical coherence
    uint8_t mappedDegree = scaleDegree;

    if (numNotes == 7 && scaleDegree < 5) {
        // For 7-note scales, map 5 frets to the most important degrees
        // Skip the 2nd and 6th degrees for a more consonant mapping
        static const uint8_t fretToScaleMap[5] = {0, 2, 3, 4, 6};  // 1, 3, 4, 5, 7
        mappedDegree = fretToScaleMap[scaleDegree];
    } else if (numNotes == 5) {
        // Direct mapping for pentatonic
        mappedDegree = scaleDegree;
    }

    // Get the interval for this scale degree
    uint8_t interval = scaleIntervals[currentScale][mappedDegree % numNotes];

    // Calculate the octave (how many times we've wrapped around the scale)
    uint8_t octaveOffset = (mappedDegree / numNotes) * 12;

    // Apply octave shift from Star Power or other controls
    int16_t totalShift = octaveOffset + (octaveShift * 12);

    // Calculate final MIDI note
    int16_t finalNote = rootNote + interval + totalShift;

    // Clamp to valid MIDI range
    if (finalNote < 0) finalNote = 0;
    if (finalNote > 127) finalNote = 127;

    return (uint8_t)finalNote;
}

const char* ScaleQuantizer::getScaleName(uint8_t scaleIndex) {
    if (scaleIndex < NUM_SCALES) {
        return scaleNames[scaleIndex];
    }
    return "Unknown";
}

uint8_t ScaleQuantizer::mapFretToScaleDegree(uint8_t fretIndex) {
    // This method can be expanded for more complex mappings
    // For now, it's a direct mapping
    return fretIndex;
}