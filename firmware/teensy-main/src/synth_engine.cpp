/**
 * Synthesizer Engine Implementation
 */

#include "synth_engine.h"

// Tone preset definitions
const SynthParams SynthEngine::presets[NUM_TONE_PRESETS] = {
    // TONE_BRIGHT - Cutting lead sound
    {
        .waveform = WAVEFORM_SAWTOOTH,
        .detune = 5.0f,
        .pulseWidth = 0.5f,
        .attack = 2.0f,
        .decay = 100.0f,
        .sustain = 0.6f,
        .release = 200.0f,
        .filterFreq = 3000.0f,
        .filterResonance = 2.5f,
        .filterEnvAmount = 0.7f,
        .reverbMix = 0.2f,
        .delayMix = 0.15f,
        .delayTime = 120.0f,
        .lfoRate = 4.0f,
        .lfoDepth = 0.1f,
        .lfoTarget = 0  // Pitch vibrato
    },

    // TONE_BALANCED - Versatile sound
    {
        .waveform = WAVEFORM_SAWTOOTH,
        .detune = 2.0f,
        .pulseWidth = 0.5f,
        .attack = 5.0f,
        .decay = 150.0f,
        .sustain = 0.7f,
        .release = 300.0f,
        .filterFreq = 2000.0f,
        .filterResonance = 2.0f,
        .filterEnvAmount = 0.5f,
        .reverbMix = 0.3f,
        .delayMix = 0.2f,
        .delayTime = 150.0f,
        .lfoRate = 3.0f,
        .lfoDepth = 0.05f,
        .lfoTarget = 1  // Filter modulation
    },

    // TONE_WARM - Mellow pad sound
    {
        .waveform = WAVEFORM_TRIANGLE,
        .detune = 1.0f,
        .pulseWidth = 0.3f,
        .attack = 10.0f,
        .decay = 200.0f,
        .sustain = 0.8f,
        .release = 500.0f,
        .filterFreq = 1200.0f,
        .filterResonance = 1.5f,
        .filterEnvAmount = 0.3f,
        .reverbMix = 0.4f,
        .delayMix = 0.25f,
        .delayTime = 200.0f,
        .lfoRate = 2.0f,
        .lfoDepth = 0.02f,
        .lfoTarget = 2  // Amplitude tremolo
    }
};

const char* SynthEngine::presetNames[NUM_TONE_PRESETS] = {
    "Bright Lead",
    "Balanced",
    "Warm Pad"
};

SynthEngine::SynthEngine() {
    currentPreset = TONE_BALANCED;
    currentParams = presets[currentPreset];
}

void SynthEngine::init() {
    // Load default preset
    loadPreset(TONE_BALANCED);
}

void SynthEngine::setTonePreset(uint8_t preset) {
    if (preset < NUM_TONE_PRESETS) {
        currentPreset = preset;
        loadPreset(preset);
    }
}

void SynthEngine::loadPreset(uint8_t preset) {
    if (preset < NUM_TONE_PRESETS) {
        currentParams = presets[preset];

        Serial.print(F("Loaded preset: "));
        Serial.println(presetNames[preset]);
    }
}

void SynthEngine::setWaveform(uint8_t waveform) {
    currentParams.waveform = waveform;
}

void SynthEngine::setFilterFreq(float freq) {
    // Clamp to reasonable range
    if (freq >= 20.0f && freq <= 20000.0f) {
        currentParams.filterFreq = freq;
    }
}

void SynthEngine::setFilterResonance(float res) {
    // Clamp to stable range
    if (res >= 0.7f && res <= 5.0f) {
        currentParams.filterResonance = res;
    }
}

void SynthEngine::setEnvelope(float a, float d, float s, float r) {
    currentParams.attack = a;
    currentParams.decay = d;
    currentParams.sustain = constrain(s, 0.0f, 1.0f);
    currentParams.release = r;
}

void SynthEngine::setReverbMix(float mix) {
    currentParams.reverbMix = constrain(mix, 0.0f, 1.0f);
}

void SynthEngine::setDelayMix(float mix) {
    currentParams.delayMix = constrain(mix, 0.0f, 1.0f);
}

void SynthEngine::setDelayTime(float time) {
    // Max delay time is typically 500ms for Teensy Audio Library
    currentParams.delayTime = constrain(time, 0.0f, 500.0f);
}

void SynthEngine::applyToVoice(AudioSynthWaveformModulated* waveform,
                                AudioEffectEnvelope* envelope,
                                AudioFilterStateVariable* filter) {
    if (!waveform || !envelope || !filter) return;

    // Apply waveform settings
    waveform->begin(currentParams.waveform);

    // Apply envelope settings
    envelope->attack(currentParams.attack);
    envelope->decay(currentParams.decay);
    envelope->sustain(currentParams.sustain);
    envelope->release(currentParams.release);

    // Apply filter settings
    filter->frequency(currentParams.filterFreq);
    filter->resonance(currentParams.filterResonance);
}

const char* SynthEngine::getPresetName(uint8_t preset) {
    if (preset < NUM_TONE_PRESETS) {
        return presetNames[preset];
    }
    return "Unknown";
}

float SynthEngine::estimateCPUUsage() {
    // Estimate CPU usage based on current parameters
    // This is a rough estimate for planning purposes

    float cpuUsage = 0.0f;

    // Base cost per voice
    cpuUsage += 2.0f;  // Oscillator

    // Waveform complexity
    switch (currentParams.waveform) {
        case WAVEFORM_SINE:
            cpuUsage += 0.5f;
            break;
        case WAVEFORM_SAWTOOTH:
        case WAVEFORM_SQUARE:
            cpuUsage += 1.0f;
            break;
        case WAVEFORM_ARBITRARY:
            cpuUsage += 2.0f;
            break;
    }

    // Filter cost
    cpuUsage += 1.5f;

    // Envelope cost
    cpuUsage += 0.5f;

    // Effects cost
    if (currentParams.reverbMix > 0) cpuUsage += 3.0f;
    if (currentParams.delayMix > 0) cpuUsage += 1.5f;

    return cpuUsage;
}

float SynthEngine::mapControlValue(float input, float inMin, float inMax, float outMin, float outMax) {
    return (input - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}