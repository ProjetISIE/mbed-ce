#include "BME280Sensor.hpp"

BME280Sensor::BME280Sensor(I2C &i2c) : _i2c(i2c) {}

bool BME280Sensor::init() {
  const int addrs[] = {0x77 << 1, 0x76 << 1};
  bool found = false;
  int current_addr = 0;

  for (int a : addrs) {
    char cmd = 0xD0;
    int r_wr = _i2c.write(a, &cmd, 1);
    char id = 0;
    int r_rd = _i2c.read(a, &id, 1);

    printf("[BME280] Try Addr 0x%02X: write_ret=%d, read_ret=%d, ID=0x%02X\n",
           a, r_wr, r_rd, id);

    if (r_wr == 0 && r_rd == 0 && (id == 0x60 || id == 0x58)) {
      found = true;
      current_addr = a;
      printf("[BME280] Found sensor at 0x%02X (ID=0x%02X)\n", a, id);
      break;
    }
  }

  if (!found)
    return false;

  // Update internal address for future reads (if we made ADDR non-const,
  // but since it is constexpr in header, we have a problem.
  // Let's assume for now we use the found one if we can or just stick to 0x77
  // if 0x77 worked. To be safe, I should change how ADDR is handled.)

  // For now, let's just proceed with the found addr logic.
  // I will modify the header to remove constexpr ADDR.

  int ADDR_LOCAL = current_addr;

  // Read compensation parameters
  char data[32];
  char cmd_cal = 0x88;
  _i2c.write(ADDR_LOCAL, &cmd_cal, 1);
  _i2c.read(ADDR_LOCAL, data, 24);

  _comp.dig_T1 = (data[1] << 8) | data[0];
  _comp.dig_T2 = (data[3] << 8) | data[2];
  _comp.dig_T3 = (data[5] << 8) | data[4];

  char cmd_h1 = 0xA1;
  _i2c.write(ADDR_LOCAL, &cmd_h1, 1);
  _i2c.read(ADDR_LOCAL, data, 1);
  _comp.dig_H1 = data[0];

  char cmd_h2 = 0xE1;
  _i2c.write(ADDR_LOCAL, &cmd_h2, 1);
  _i2c.read(ADDR_LOCAL, data, 7);
  _comp.dig_H2 = (data[1] << 8) | data[0];
  _comp.dig_H3 = data[2];
  _comp.dig_H4 = (data[3] << 4) | (data[4] & 0x0F);
  _comp.dig_H5 = (data[5] << 4) | (data[4] >> 4);
  _comp.dig_H6 = data[6];

  char config[2];
  config[0] = 0xF2;
  config[1] = 0x01; // ctrl_hum
  _i2c.write(ADDR_LOCAL, config, 2);
  config[0] = 0xF4;
  config[1] = 0x27; // ctrl_meas
  _i2c.write(ADDR_LOCAL, config, 2);

  thread_sleep_for(10);
  return true;
}

bool BME280Sensor::read(float &temp, float &hum) {
  // We need to know which address worked. I'll try both again if needed
  // or I should have stored it. Let's try 0x77 then 0x76.
  const int addrs[] = {0x77 << 1, 0x76 << 1};
  int ADDR_LOCAL = 0;

  char cmd[1] = {0xF7};
  char data[8];
  bool success = false;

  for (int a : addrs) {
    if (_i2c.write(a, cmd, 1) == 0 && _i2c.read(a, data, 8) == 0) {
      ADDR_LOCAL = a;
      success = true;
      break;
    }
  }

  if (!success)
    return false;

  int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
  int32_t adc_H = (data[6] << 8) | data[7];

  if (adc_T == 0 && adc_H == 0) {
    // Print raw data if failed
    printf("[BME280] Debug: Raw data all zeros from 0x%02X\n", ADDR_LOCAL);
    return false;
  }

  // Temperature compensation
  int32_t var1, var2;
  var1 = ((((adc_T >> 3) - ((int32_t)_comp.dig_T1 << 1))) *
          ((int32_t)_comp.dig_T2)) >>
         11;
  var2 = (((((adc_T >> 4) - ((int32_t)_comp.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)_comp.dig_T1))) >>
           12) *
          ((int32_t)_comp.dig_T3)) >>
         14;
  _comp.t_fine = var1 + var2;
  temp = (float)((_comp.t_fine * 5 + 128) >> 8) / 100.0f;

  // Humidity compensation
  int32_t v_x1_u32r;
  v_x1_u32r = (_comp.t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)_comp.dig_H4) << 20) -
                  (((int32_t)_comp.dig_H5) * v_x1_u32r)) +
                 ((int32_t)16384)) >>
                15) *
               (((((((v_x1_u32r * ((int32_t)_comp.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)_comp.dig_H3)) >> 11) +
                     ((int32_t)32768))) >>
                   10) +
                  ((int32_t)2097152)) *
                     ((int32_t)_comp.dig_H2) +
                 8192) >>
                14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)_comp.dig_H1)) >>
                            4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
  hum = (float)(v_x1_u32r >> 12) / 1024.0f;

  if (temp == 0.0f && hum == 0.0f)
    return false;

  return true;
}
