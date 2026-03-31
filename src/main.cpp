#include "BME280Sensor.hpp"
#include "PAM8302.hpp"
#include "Synth.hpp"
#include "TH02Sensor.hpp"
#include "mbed.h"
#include <cmath>

// PINs Definitions (LPC1768)
#define SYNTH_I2C_SDA p9
#define SYNTH_I2C_SCL p10
#define AUD_OUT p18
#define AMP_SD p15

int main() {
  printf("Temperature & Humidity Synth starting\n");
  I2C i2c(SYNTH_I2C_SDA, SYNTH_I2C_SCL);
  i2c.frequency(100000);
  TH02Sensor th02(i2c);
  BME280Sensor bme(i2c);
  PAM8302 amp(AMP_SD);
  bool th02_ok = th02.init();
  bool bme_ok = bme.init();
  if (!th02_ok)
    printf("WARN: TH02 initialization failed\n");
  if (!bme_ok)
    printf("WARN: BME280 initialization failed\n");

  Synth synth(AUD_OUT);
  amp.on();
  float base_temp = 20.0f;
  float base_hum = 40.0f;
  printf("Calibrating baselines, don’t touch\n");

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
    } else
      printf("  Reading %d: FAILED\n", i + 1);
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
    printf("TH02: ");
    if (r_th02) {
      current_temp = t_th02;
      current_hum = h_th02;
      printf("%.1f C, %.1f %% | ", t_th02, h_th02);
    } else
      printf("ERR | ");
    printf("BME280: ");
    if (r_bme) {
      printf("%.1f C, %.1f %%", t_bme, h_bme);
      if (!r_th02) {
        current_temp = t_bme;
        current_hum = h_bme;
      }
    } else
      printf("ERR");
    printf("\n");

    // Frequency control
    float temp_diff = current_temp - base_temp;
    float threshold = 0.5f;
    float freq = 0.0f;
    float amplitude = 0.0f;
    float mod_rate = 0.0f;

    if (temp_diff > threshold) {
      float octave_shift = 0.5f * (temp_diff - threshold);
      freq = 110.0f * std::pow(2.0f, octave_shift);
      synth.set_frequency(freq);
      float hum_diff = current_hum - base_hum;
      amplitude = 0.5f + (hum_diff / 50.0f);
      synth.set_amplitude(amplitude);
      mod_rate = 1.0f + (hum_diff / 10.0f);
      if (mod_rate < 0.1f)
        mod_rate = 0.1f;
      synth.set_mod_rate(mod_rate);
    } else
      synth.set_amplitude(0.0f);

    amp.log_status(freq, amplitude, mod_rate);
    thread_sleep_for(500); // 2Hz updates
  }
}
