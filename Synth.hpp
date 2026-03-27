#ifndef SYNTH_H
#define SYNTH_H

#include "mbed.h"

class Synth {
public:
  Synth(PinName out_pin);
  void start();
  void set_frequency(float freq);
  void set_amplitude(float amp);
  void set_mod_rate(float rate);

private:
  void sample_tick();

  AnalogOut _audio_out;
  Ticker _sample_ticker;
  float _phase;
  float _phase_increment;
  float _mod_phase;
  float _mod_phase_increment;
  float _amplitude;
  float _frequency;

  static constexpr float SAMPLE_RATE = 22050.0f;
  static constexpr float PI = 3.14159265359f;
};

#endif // SYNTH_H
