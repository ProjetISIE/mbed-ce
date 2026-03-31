#include "BME280Sensor.hpp"

BME280Sensor::BME280Sensor(I2C &i2c) : _i2c(i2c) {}

bool BME280Sensor::init() {
  char cmd[2];
  // Read ID
  cmd[0] = 0xD0;
  if (_i2c.write(ADDR, cmd, 1) != 0)
    return false;
  if (_i2c.read(ADDR, cmd, 1) != 0)
    return false;
  // BME280 is 0x60, BMP280 is 0x58
  if (cmd[0] != 0x60 && cmd[0] != 0x58)
    return false;

  // Read compensation parameters
  char data[32];
  cmd[0] = 0x88;
  _i2c.write(ADDR, cmd, 1);
  _i2c.read(ADDR, data, 24);

  _comp.dig_T1 = (data[1] << 8) | data[0];
  _comp.dig_T2 = (data[3] << 8) | data[2];
  _comp.dig_T3 = (data[5] << 8) | data[4];

  cmd[0] = 0xA1;
  _i2c.write(ADDR, cmd, 1);
  _i2c.read(ADDR, data, 1);
  _comp.dig_H1 = data[0];

  cmd[0] = 0xE1;
  _i2c.write(ADDR, cmd, 1);
  _i2c.read(ADDR, data, 7);
  _comp.dig_H2 = (data[1] << 8) | data[0];
  _comp.dig_H3 = data[2];
  _comp.dig_H4 = (data[3] << 4) | (data[4] & 0x0F);
  _comp.dig_H5 = (data[5] << 4) | (data[4] >> 4);
  _comp.dig_H6 = data[6];

  // Config: hum oversampling x1, temp x1, normal mode
  cmd[0] = 0xF2;
  cmd[1] = 0x01; // ctrl_hum
  _i2c.write(ADDR, cmd, 2);
  cmd[0] = 0xF4;
  cmd[1] = 0x27; // ctrl_meas
  _i2c.write(ADDR, cmd, 2);

  thread_sleep_for(10); // Wait for first measurement
  return true;
}

bool BME280Sensor::read(float &temp, float &hum) {
  char cmd[1] = {0xF7};
  char data[8];
  if (_i2c.write(ADDR, cmd, 1) != 0)
    return false;
  if (_i2c.read(ADDR, data, 8) != 0)
    return false;

  int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
  int32_t adc_H = (data[6] << 8) | data[7];

  if (adc_T == 0 || adc_H == 0)
    return false;

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
