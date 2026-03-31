#ifndef PAM8302_HPP
#define PAM8302_HPP

#include "Amplifier.hpp"
#include "mbed.h"

class PAM8302 : public Amplifier {
public:
  PAM8302(PinName sd_pin);
  void on() override;
  void off() override;
  void log_status(float freq, float amp, float mod_rate) override;

private:
  DigitalOut _sd_pin;
  bool _is_on;
};

#endif // PAM8302_HPP
