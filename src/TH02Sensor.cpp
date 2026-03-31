#include "TH02Sensor.hpp"

TH02Sensor::TH02Sensor(I2C &i2c) : _i2c(i2c) {}

bool TH02Sensor::init() {
  char cmd = 0x11; // REG_ID
  char data = 0;
  int r_wr = 1, r_rd = 1;

  for (int i = 0; i < 3; i++) {
    r_wr = _i2c.write(ADDR, &cmd, 1);
    if (r_wr == 0) {
      wait_us(1000); // Settling delay
      r_rd = _i2c.read(ADDR, &data, 1);
      if (r_rd == 0)
        break;
    }
    thread_sleep_for(10);
  }

  printf("[TH02] Init: write_ret=%d, read_ret=%d, ID=0x%02X\n", r_wr, r_rd,
         data);
  return (r_wr == 0 && r_rd == 0);
}

bool TH02Sensor::read(float &temp, float &hum) {
  char cmd[2];
  char data[3];

  // Measure Temperature
  cmd[0] = 0x03; // REG_CONFIG
  cmd[1] = 0x11; // CMD_MEASURE_TEMP

  bool success = false;
  for (int i = 0; i < 3; i++) {
    if (_i2c.write(ADDR, cmd, 2) == 0) {
      success = true;
      break;
    }
    thread_sleep_for(10);
  }
  if (!success) {
    printf("[TH02] Read: Write config (temp) failed after retries\n");
    return false;
  }

  bool ready = false;
  for (int i = 0; i < 20; i++) {
    thread_sleep_for(20);
    cmd[0] = 0x00; // REG_STATUS
    if (_i2c.write(ADDR, cmd, 1) == 0) {
      wait_us(1000);
      if (_i2c.read(ADDR, data, 1) == 0) {
        if (!(data[0] & 0x01)) {
          ready = true;
          break;
        }
      }
    }
  }
  if (!ready) {
    printf("[TH02] Read: Timeout waiting for temp ready\n");
    return false;
  }

  cmd[0] = 0x01; // REG_DATA_H
  if (_i2c.write(ADDR, cmd, 1) == 0) {
    wait_us(1000);
    if (_i2c.read(ADDR, data, 3) != 0) {
      printf("[TH02] Read: Read data (temp) failed\n");
      return false;
    }
  } else {
    return false;
  }

  uint16_t temp_raw = (data[1] << 8) | data[2];
  temp = (float)(temp_raw >> 2) / 32.0f - 50.0f;

  // Measure Humidity
  cmd[0] = 0x03; // REG_CONFIG
  cmd[1] = 0x01; // CMD_MEASURE_HUMI
  success = false;
  for (int i = 0; i < 3; i++) {
    if (_i2c.write(ADDR, cmd, 2) == 0) {
      success = true;
      break;
    }
    thread_sleep_for(10);
  }
  if (!success) {
    printf("[TH02] Read: Write config (hum) failed after retries\n");
    return false;
  }

  ready = false;
  for (int i = 0; i < 20; i++) {
    thread_sleep_for(20);
    cmd[0] = 0x00; // REG_STATUS
    if (_i2c.write(ADDR, cmd, 1) == 0) {
      wait_us(1000);
      if (_i2c.read(ADDR, data, 1) == 0) {
        if (!(data[0] & 0x01)) {
          ready = true;
          break;
        }
      }
    }
  }
  if (!ready) {
    printf("[TH02] Read: Timeout waiting for hum ready\n");
    return false;
  }

  cmd[0] = 0x01; // REG_DATA_H
  if (_i2c.write(ADDR, cmd, 1) == 0) {
    wait_us(1000);
    if (_i2c.read(ADDR, data, 3) != 0) {
      printf("[TH02] Read: Read data (hum) failed\n");
      return false;
    }
  } else {
    return false;
  }

  uint16_t humi_raw = (data[1] << 8) | data[2];
  hum = (float)(humi_raw >> 4) / 16.0f - 24.0f;

  if (temp == 0.0f && hum == 0.0f) {
    printf("[TH02] Read: All zeros received\n");
    return false;
  }

  return true;
}
