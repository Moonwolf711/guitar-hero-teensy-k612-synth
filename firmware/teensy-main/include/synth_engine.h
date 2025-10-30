/**
 * Synthesizer Engine Module
 * Manages synthesis parameters and tone presets
 */

#ifndef SYNTH_ENGINE_H
#define SYNTH_ENGINE_H

#include <Arduino.h>
#include <Audio.h>

// Tone presets
enum TonePreset {
    TONE_BRIGHT = 0,
    TONE_BALANCED,
    TONE_WARM,
    NUM_TONE_PRESETS
};

// Synthesis parameters structure
struct SynthParams {
    // Oscillator parameters
    uint8_t waveform;        // WAVEFORM_SINE, WAVEFORM_SAWTOOTH, etc.
    float detune;            // Detune amount in cents
    float pulseWidth;        // For pulse wave (0.0 - 1.0)

    // Envelope parameters (ADSR in milliseconds)
    float attack;
    float decay;
    float sustain;          // 0.0 - 1.0
    float release;

    // Filter parameters
    float filterFreq;        // Hz
    float filterResonance;   // 0.7 - 5.0
    float filterEnvAmount;   // How much envelope affects filter

    // Effects levels
    float reverbMix;        // 0.0 - 1.0
    float delayMix;         // 0.0 - 1.0
    float delayTime;        // milliseconds

    // Modulation
    float lfoRate;          // Hz
    float lfoDepth;         // 0.0 - 1.0
    uint8_t lfoTarget;      // 0=pitch, 1=filter, 2=amplitude
};

class SynthEngine {
public:
    SynthEngine();

    // Initialize the synthesis engine
    void init();

    // Set tone preset
    void setTonePreset(uint8_t preset);

    // Get current parameters
    SynthParams& getParams() { return currentParams; }

    // Set individual parameters
    void setWaveform(uint8_t waveform);
    void setFilterFreq(float freq);
    void setFilterResonance(float res);
    void setEnvelope(float a, float d, float s, float r);
    void setReverbMix(float mix);
    void setDelayMix(float mix);
    void setDelayTime(float time);

    // Apply parameters to audio objects
    void applyToVoice(AudioSynthWaveformModulated* waveform,
                      AudioEffectEnvelope* envelope,
                      AudioFilterStateVariable* filter);

    // Get preset name
    const char* getPresetName(uint8_t preset);

    // Performance optimization
    float estimateCPUUsage();

private:
    SynthParams currentParams;
    uint8_t currentPreset;

    // Preset definitions
    static const SynthParams presets[NUM_TONE_PRESETS];
    static const char* presetNames[NUM_TONE_PRESETS];

    // Helper functions
    void loadPreset(uint8_t preset);
    float mapControlValue(float input, float inMin, float inMax, float outMin, float outMax);
};

#endif // SYNTH_ENGINE_H