#include "PAM8302.hpp"

PAM8302::PAM8302(PinName sd_pin) : _sd_pin(sd_pin, 1), _is_on(true) {
  // PAM8302 SD (Shutdown) pin is active Low (pull Low to shutdown)
  // We initialize it to High (1) to be On by default.
}

void PAM8302::on() {
  _sd_pin = 1;
  _is_on = true;
}

void PAM8302::off() {
  _sd_pin = 0;
  _is_on = false;
}

void PAM8302::log_status(float freq, float amp, float mod_rate) {
  if (!_is_on || amp <= 0.0f) {
    printf("[Amplifier] Status: Silent\n");
  } else {
    printf("[Amplifier] Status: Freq=%.1f Hz, Amp=%.2f, Mod=%.1f Hz\n", freq,
           amp, mod_rate);
  }
}
