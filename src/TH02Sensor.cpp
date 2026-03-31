#include "TH02Sensor.hpp"

TH02Sensor::TH02Sensor(I2C &i2c) : _i2c(i2c) {}

bool TH02Sensor::init() {
  // TH02 usually doesn't need specific init beyond basic check
  char cmd = 0x11; // REG_ID
  char data;
  if (_i2c.write(ADDR, &cmd, 1) != 0)
    return false;
  if (_i2c.read(ADDR, &data, 1) != 0)
    return false;
  return true;
}

bool TH02Sensor::read(float &temp, float &hum) {
  char cmd[2];
  char data[3];

  // Measure Temperature
  cmd[0] = 0x03; // REG_CONFIG
  cmd[1] = 0x11; // CMD_MEASURE_TEMP
  if (_i2c.write(ADDR, cmd, 2) != 0)
    return false;

  bool ready = false;
  for (int i = 0; i < 20; i++) {
    thread_sleep_for(20);
    cmd[0] = 0x00; // REG_STATUS
    if (_i2c.write(ADDR, cmd, 1) != 0)
      break;
    if (_i2c.read(ADDR, data, 1) != 0)
      break;
    if (!(data[0] & 0x01)) {
      ready = true;
      break;
    }
  }
  if (!ready)
    return false;

  cmd[0] = 0x01; // REG_DATA_H
  if (_i2c.write(ADDR, cmd, 1) != 0)
    return false;
  if (_i2c.read(ADDR, data, 3) != 0)
    return false;
  uint16_t temp_raw = (data[1] << 8) | data[2];
  temp = (float)(temp_raw >> 2) / 32.0f - 50.0f;

  // Measure Humidity
  cmd[0] = 0x03; // REG_CONFIG
  cmd[1] = 0x01; // CMD_MEASURE_HUMI
  if (_i2c.write(ADDR, cmd, 2) != 0)
    return false;

  ready = false;
  for (int i = 0; i < 20; i++) {
    thread_sleep_for(20);
    cmd[0] = 0x00; // REG_STATUS
    if (_i2c.write(ADDR, cmd, 1) != 0)
      break;
    if (_i2c.read(ADDR, data, 1) != 0)
      break;
    if (!(data[0] & 0x01)) {
      ready = true;
      break;
    }
  }
  if (!ready)
    return false;

  cmd[0] = 0x01; // REG_DATA_H
  if (_i2c.write(ADDR, cmd, 1) != 0)
    return false;
  if (_i2c.read(ADDR, data, 3) != 0)
    return false;
  uint16_t humi_raw = (data[1] << 8) | data[2];
  hum = (float)(humi_raw >> 4) / 16.0f - 24.0f;

  return true;
}
