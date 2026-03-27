#include "BME280Sensor.hpp"
#include "Synth.hpp"
#include "TH02Sensor.hpp"
#include "mbed.h"
#include <cmath>

// Pin definitions for LPC1768
#define SYNTH_I2C_SDA p9
#define SYNTH_I2C_SCL p10
#define AUDIO_OUT p18

int main() {
  printf("Temperature & Humidity Synth starting...\n");

  I2C i2c(SYNTH_I2C_SDA, SYNTH_I2C_SCL);
  i2c.frequency(100000);

  TH02Sensor th02(i2c);
  BME280Sensor bme(i2c);

  bool th02_ok = th02.init();
  bool bme_ok = bme.init();

  if (!th02_ok)
    printf("Warning: TH02 initialization failed\n");
  if (!bme_ok)
    printf("Warning: BME280 initialization failed\n");

  Synth synth(AUDIO_OUT);

  // Calibration at boot
  float base_temp = 20.0f;
  float base_hum = 40.0f;
  printf("Calibrating baseline... Keep it steady.\n");

  for (int i = 0; i < 5; i++) {
    float t_th02, h_th02, t_bme, h_bme;
    bool r_th02 = th02.read(t_th02, h_th02);
    bool r_bme = bme.read(t_bme, h_bme);

    if (r_th02) {
      base_temp = t_th02;
      base_hum = h_th02;
      printf("  [TH02] Reading %d: %.2f C, %.2f %%\n", i + 1, t_th02, h_th02);
    } else if (r_bme) {
      base_temp = t_bme;
      base_hum = h_bme;
      printf("  [BME280] Reading %d: %.2f C, %.2f %%\n", i + 1, t_bme, h_bme);
    } else {
      printf("  Reading %d: FAILED\n", i + 1);
    }
    thread_sleep_for(200);
  }
  printf("Baseline Final: Temp = %.2f C, Hum = %.2f %%\n", base_temp, base_hum);

  synth.start();
  synth.set_amplitude(0.0f);

  while (true) {
    float t_th02, h_th02, t_bme, h_bme;
    bool r_th02 = th02.read(t_th02, h_th02);
    bool r_bme = bme.read(t_bme, h_bme);

    float current_temp = base_temp;
    float current_hum = base_hum;

    if (r_th02) {
      current_temp = t_th02;
      current_hum = h_th02;
      printf("TH02: %.2f C, %.2f %% | ", t_th02, h_th02);
    } else {
      printf("TH02: ERR | ");
    }

    if (r_bme) {
      printf("BME280: %.2f C, %.2f %%", t_bme, h_bme);
      // If TH02 failed, use BME280 for synth
      if (!r_th02) {
        current_temp = t_bme;
        current_hum = h_bme;
      }
    } else {
      printf("BME280: ERR");
    }
    printf("\n");

    // Frequency control:
    float temp_diff = current_temp - base_temp;
    float threshold = 0.5f;

    if (temp_diff > threshold) {
      float octave_shift = 0.5f * (temp_diff - threshold);
      float freq = 110.0f * std::pow(2.0f, octave_shift);
      synth.set_frequency(freq);

      float hum_diff = current_hum - base_hum;
      float amp = 0.5f + (hum_diff / 50.0f);
      synth.set_amplitude(amp);

      float mod_rate = 1.0f + (hum_diff / 10.0f);
      if (mod_rate < 0.1f)
        mod_rate = 0.1f;
      synth.set_mod_rate(mod_rate);
    } else {
      synth.set_amplitude(0.0f);
    }

    thread_sleep_for(200);
  }
}
