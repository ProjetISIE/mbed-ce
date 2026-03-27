#include "Sensors.hpp"
#include "Synth.hpp"
#include "mbed.h"
#include <cmath>

// Pin definitions for LPC1768
#define SYNTH_I2C_SDA p9
#define SYNTH_I2C_SCL p10
#define AUDIO_OUT p18

int main() {
  printf("Temperature & Humidity Synth starting...\n");

  Synth synth(AUDIO_OUT);
  Sensors sensors(SYNTH_I2C_SDA, SYNTH_I2C_SCL);

  if (!sensors.init()) {
    printf("Warning: Some sensors failed to initialize\n");
  }

  // Calibration at boot
  float base_temp = 20.0f; // Default room temp
  float base_hum = 40.0f;  // Default humidity
  printf("Calibrating baseline... Keep it steady.\n");

  // Take a few readings to stabilize
  for (int i = 0; i < 5; i++) {
    float t, h;
    sensors.get_average_temp_and_humidity(t, h);
    // 0.0 is often a sign of a bad read or uninitialized sensor in this context
    if (t > -50.0f && t != 0.0f) {
      base_temp = t;
      base_hum = h;
      printf("  Reading %d: %.2f C, %.2f %% (OK)\n", i + 1, t, h);
    } else {
      printf("  Reading %d: %.2f C, %.2f %% (INVALID, using %.2f C)\n", i + 1,
             t, h, base_temp);
    }
    thread_sleep_for(200);
  }

  printf("Baseline Final: Temp = %.2f C, Hum = %.2f %%\n", base_temp, base_hum);

  synth.start();
  synth.set_amplitude(0.0f); // Silent at start

  while (true) {
    float current_temp, current_hum;
    sensors.get_average_temp_and_humidity(current_temp, current_hum);

    // Frequency control:
    // A slight increase in temperature should produce an A2 note (110Hz)
    // Increase pitch at about half an octave per °C.
    // Formula: freq = 110 * 2^(0.5 * (current_temp - (base_temp + threshold)))

    float temp_diff = current_temp - base_temp;
    float threshold = 0.5f; // Threshold to start producing sound

    if (temp_diff > threshold) {
      float octave_shift = 0.5f * (temp_diff - threshold);
      float freq = 110.0f * std::pow(2.0f, octave_shift);
      synth.set_frequency(freq);

      // Amplitude control from humidity:
      float hum_diff = current_hum - base_hum;
      float amp = 0.5f + (hum_diff / 50.0f);
      synth.set_amplitude(amp);

      // Modulation rate (LFO) also from humidity:
      // Let's say 1Hz at base, increases with humidity
      float mod_rate = 1.0f + (hum_diff / 10.0f);
      if (mod_rate < 0.1f)
        mod_rate = 0.1f;
      synth.set_mod_rate(mod_rate);
    } else {
      synth.set_amplitude(0.0f);
    }

    printf("Temp: %.2f C (Base: %.2f), Hum: %.2f %% (Base: %.2f)\n",
           current_temp, base_temp, current_hum, base_hum);

    thread_sleep_for(100); // Update at 10Hz
  }
}
