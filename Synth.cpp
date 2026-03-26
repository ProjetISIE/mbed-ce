#include "Synth.h"
#include <cmath>

Synth::Synth(PinName out_pin) : _audio_out(out_pin), _phase(0.0f), _mod_phase(0.0f), _amplitude(0.0f), _frequency(0.0f) {
    _phase_increment = 0.0f;
    _mod_phase_increment = 0.0f;
}

void Synth::start() {
    _sample_ticker.attach(callback(this, &Synth::sample_tick), std::chrono::microseconds(static_cast<int>(1000000.0f / SAMPLE_RATE)));
}

void Synth::set_frequency(float freq) {
    _frequency = freq;
    _phase_increment = 2.0f * PI * _frequency / SAMPLE_RATE;
}

void Synth::set_amplitude(float amp) {
    _amplitude = (amp < 0.0f) ? 0.0f : ((amp > 1.0f) ? 1.0f : amp);
}

void Synth::set_mod_rate(float rate) {
    _mod_phase_increment = 2.0f * PI * rate / SAMPLE_RATE;
}

void Synth::sample_tick() {
    if (_amplitude <= 0.0f) {
        _audio_out.write(0.5f); // Center bias
        return;
    }

    _phase += _phase_increment;
    if (_phase >= 2.0f * PI) _phase -= 2.0f * PI;

    _mod_phase += _mod_phase_increment;
    if (_mod_phase >= 2.0f * PI) _mod_phase -= 2.0f * PI;

    // Amplitude modulation with LFO
    float mod = 0.7f + 0.3f * std::sin(_mod_phase);
    float current_amp = _amplitude * mod;

    // Sine wave shifted from [-1, 1] to [0, 1] for DAC
    float sample = 0.5f + 0.5f * current_amp * std::sin(_phase);
    _audio_out.write(sample);
}
