#include "BME280Sensor.hpp"

BME280Sensor::BME280Sensor(I2C &i2c) : _i2c(i2c), _address(0) {}

bool BME280Sensor::init() {
  const int addrs[] = {0x77 << 1, 0x76 << 1};
  bool found = false;

  for (int a : addrs) {
    char cmd = 0xD0;
    char id = 0;
    int r_wr = 1, r_rd = 1;

    for (int i = 0; i < 3; i++) {
      r_wr = _i2c.write(a, &cmd, 1);
      if (r_wr == 0) {
        wait_us(1000); // Settling delay
        r_rd = _i2c.read(a, &id, 1);
        if (r_rd == 0)
          break;
      }
      thread_sleep_for(10);
    }

    printf("[BME280] Try Addr 0x%02X: write_ret=%d, read_ret=%d, ID=0x%02X\n",
           a, r_wr, r_rd, id);

    if (r_wr == 0 && r_rd == 0 && (id == 0x60 || id == 0x58)) {
      found = true;
      _address = a;
      printf("[BME280] Found sensor at 0x%02X (ID=0x%02X)\n", a, id);
      break;
    }
  }

  if (!found)
    return false;

  // Read compensation parameters
  char data[32];
  char cmd_cal = 0x88;
  if (_i2c.write(_address, &cmd_cal, 1) == 0) {
    wait_us(1000);
    if (_i2c.read(_address, data, 24) != 0)
      return false;
  } else {
    return false;
  }

  _comp.dig_T1 = (data[1] << 8) | data[0];
  _comp.dig_T2 = (data[3] << 8) | data[2];
  _comp.dig_T3 = (data[5] << 8) | data[4];

  char cmd_h1 = 0xA1;
  if (_i2c.write(_address, &cmd_h1, 1) == 0) {
    wait_us(1000);
    if (_i2c.read(_address, data, 1) != 0)
      return false;
  } else {
    return false;
  }
  _comp.dig_H1 = data[0];

  char cmd_h2 = 0xE1;
  if (_i2c.write(_address, &cmd_h2, 1) == 0) {
    wait_us(1000);
    if (_i2c.read(_address, data, 7) != 0)
      return false;
  } else {
    return false;
  }
  _comp.dig_H2 = (data[1] << 8) | data[0];
  _comp.dig_H3 = data[2];
  _comp.dig_H4 = (data[3] << 4) | (data[4] & 0x0F);
  _comp.dig_H5 = (data[5] << 4) | (data[4] >> 4);
  _comp.dig_H6 = data[6];

  char config[2];
  config[0] = 0xF2;
  config[1] = 0x01; // ctrl_hum
  _i2c.write(_address, config, 2);
  wait_us(1000);
  config[0] = 0xF4;
  config[1] = 0x27; // ctrl_meas
  _i2c.write(_address, config, 2);

  thread_sleep_for(20);
  return true;
}

bool BME280Sensor::read(float &temp, float &hum) {
  if (_address == 0)
    return false;

  char cmd[1] = {0xF7};
  char data[8];

  bool success = false;
  for (int i = 0; i < 3; i++) {
    if (_i2c.write(_address, cmd, 1) == 0) {
      wait_us(1000);
      if (_i2c.read(_address, data, 8) == 0) {
        success = true;
        break;
      }
    }
    thread_sleep_for(10);
  }

  if (!success)
    return false;

  int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
  int32_t adc_H = (data[6] << 8) | data[7];

  if (adc_T == 0 && adc_H == 0) {
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
