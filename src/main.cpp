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
  i2c.frequency(10000); // 10kHz for stability

  printf("I2C Scanner: Scanning (skipping 0x00)...\n");
  int count = 0;
  for (int address = 2; address < 256; address += 2) {
    if (i2c.write(address, NULL, 0) == 0) {
      printf("  - Found device at 0x%02X (7-bit: 0x%02X)\n", address,
             address >> 1);
      count++;
    }
  }
  printf("I2C Scanner: Found %d devices.\n", count);

  TH02Sensor th02(i2c);
  BME280Sensor bme(i2c);
  PAM8302 amp(AMP_SD);

  thread_sleep_for(100);
  bool th02_ok = th02.init();
  thread_sleep_for(100);
  bool bme_ok = bme.init();
  thread_sleep_for(100);

  if (!th02_ok)
    printf("WARN: TH02 initialization failed\n");
  if (!bme_ok)
    printf("WARN: BME280 initialization failed\n");

  Synth synth(AUD_OUT);
  float base_temp = 20.0f;
  float base_hum = 40.0f;
  bool baseline_found = false;
  printf(
      "Calibrating baselines, don’t touch (Need at least one valid reading)\n");

  for (int i = 0; i < 5; i++) {
    float t_th02, h_th02, t_bme, h_bme;
    bool r_th02 = false;
    if (th02_ok) {
      thread_sleep_for(50);
      r_th02 = th02.read(t_th02, h_th02);
    }
    bool r_bme = false;
    if (bme_ok) {
      thread_sleep_for(50);
      r_bme = bme.read(t_bme, h_bme);
    }

    // Reject all-zero readings as failures
    if (r_th02 && (t_th02 == 0.0f && h_th02 == 0.0f))
      r_th02 = false;
    if (r_bme && (t_bme == 0.0f && h_bme == 0.0f))
      r_bme = false;

    if (r_th02) {
      base_temp = t_th02;
      base_hum = h_th02;
      baseline_found = true;
      printf("  [TH02] Reading %d: %.2f C, %.2f %%\n", i + 1, t_th02, h_th02);
    } else if (r_bme) {
      base_temp = t_bme;
      base_hum = h_bme;
      baseline_found = true;
      printf("  [BME280] Reading %d: %.2f C, %.2f %%\n", i + 1, t_bme, h_bme);
    } else {
      printf("  Reading %d: FAILED\n", i + 1);
    }
    thread_sleep_for(200);
  }

  if (baseline_found) {
    printf("Baseline Final: Temp = %.2f C, Hum = %.2f %%\n", base_temp,
           base_hum);
    synth.start();
    amp.on();
    synth.set_amplitude(0.0f);
  } else {
    printf("WARN: No baseline found, waiting for valid data...\n");
  }

  bool synth_started = baseline_found;

  while (true) {
    float t_th02, h_th02, t_bme, h_bme;
    bool r_th02 = false;
    if (th02_ok) {
      thread_sleep_for(50);
      r_th02 = th02.read(t_th02, h_th02);
    }
    bool r_bme = false;
    if (bme_ok) {
      thread_sleep_for(50);
      r_bme = bme.read(t_bme, h_bme);
    }

    if (r_th02 && (t_th02 == 0.0f && h_th02 == 0.0f))
      r_th02 = false;
    if (r_bme && (t_bme == 0.0f && h_bme == 0.0f))
      r_bme = false;

    float current_temp = base_temp;
    float current_hum = base_hum;
    bool current_ok = r_th02 || r_bme;

    if (!synth_started && current_ok) {
      if (r_th02) {
        base_temp = t_th02;
        base_hum = h_th02;
      } else {
        base_temp = t_bme;
        base_hum = h_bme;
      }
      printf("First valid reading! Starting audio with baseline: Temp = %.2f "
             "C, Hum = %.2f %%\n",
             base_temp, base_hum);
      synth.start();
      amp.on();
      synth_started = true;
      current_temp = base_temp;
      current_hum = base_hum;
    }

    printf("TH02: ");
    if (r_th02) {
      current_temp = t_th02;
      current_hum = h_th02;
      printf("%.1f C, %.1f %% | ", t_th02, h_th02);
    } else {
      printf("ERR | ");
    }

    printf("BME280: ");
    if (r_bme) {
      printf("%.1f C, %.1f %%", t_bme, h_bme);
      if (!r_th02) {
        current_temp = t_bme;
        current_hum = h_bme;
      }
    } else {
      printf("ERR");
    }
    printf("\n");

    // Frequency control
    float temp_diff = current_temp - base_temp;
    float threshold = 0.5f;
    float freq = 0.0f;
    float amplitude = 0.0f;
    float mod_rate = 0.0f;

    if (synth_started && current_ok && temp_diff > threshold) {
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
    } else if (synth_started) {
      synth.set_amplitude(0.0f);
    }

    if (synth_started)
      amp.log_status(freq, amplitude, mod_rate);
    else
      printf("[System] Waiting for initial data...\n");

    thread_sleep_for(500); // 2Hz updates
  }
}
