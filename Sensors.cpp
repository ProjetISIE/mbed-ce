#include "Sensors.hpp"

Sensors::Sensors(PinName sda, PinName scl) : _i2c(sda, scl) {
  _i2c.frequency(100000);
}

bool Sensors::init() {
  printf("Scanning I2C bus (p9/p10)...\n");
  int count = 0;
  for (int address = 1; address < 127; address++) {
    if (_i2c.write(address << 1, NULL, 0) == 0) {
      printf("  Found device at 0x%02X\n", address);
      count++;
    }
  }
  if (count == 0) {
    printf("  No I2C devices found! Check wiring and pull-ups.\n");
  }

  bool ok = true;
  if (!init_sht31()) {
    printf("SHT31 initialization failed\n");
    ok = false;
  }
  if (!init_bme280()) {
    printf("BME280 initialization failed\n");
    ok = false;
  }
  return ok;
}

bool Sensors::init_sht31() {
  // SHT31 doesn't need much initialization for one-shot reading
  return true;
}

bool Sensors::init_bme280() {
  char cmd[2];
  // Read ID
  cmd[0] = 0xD0;
  _i2c.write(BME280_ADDR, cmd, 1);
  _i2c.read(BME280_ADDR, cmd, 1);
  if (cmd[0] != 0x60)
    return false;

  // Read compensation parameters
  char data[32];
  cmd[0] = 0x88;
  _i2c.write(BME280_ADDR, cmd, 1);
  _i2c.read(BME280_ADDR, data, 24);

  _bme280_comp.dig_T1 = (data[1] << 8) | data[0];
  _bme280_comp.dig_T2 = (data[3] << 8) | data[2];
  _bme280_comp.dig_T3 = (data[5] << 8) | data[4];

  cmd[0] = 0xA1;
  _i2c.write(BME280_ADDR, cmd, 1);
  _i2c.read(BME280_ADDR, data, 1);
  _bme280_comp.dig_H1 = data[0];

  cmd[0] = 0xE1;
  _i2c.write(BME280_ADDR, cmd, 1);
  _i2c.read(BME280_ADDR, data, 7);
  _bme280_comp.dig_H2 = (data[1] << 8) | data[0];
  _bme280_comp.dig_H3 = data[2];
  _bme280_comp.dig_H4 = (data[3] << 4) | (data[4] & 0x0F);
  _bme280_comp.dig_H5 = (data[5] << 4) | (data[4] >> 4);
  _bme280_comp.dig_H6 = data[6];

  // Config: hum oversampling x1, temp x1, normal mode
  cmd[0] = 0xF2;
  cmd[1] = 0x01; // ctrl_hum
  _i2c.write(BME280_ADDR, cmd, 2);
  cmd[0] = 0xF4;
  cmd[1] = 0x27; // ctrl_meas
  _i2c.write(BME280_ADDR, cmd, 2);

  return true;
}

bool Sensors::read_sht31(float &temp, float &hum) {
  char cmd[2] = {0x24, 0x00}; // High repeatability, clock stretching disabled
  if (_i2c.write(SHT31_ADDR, cmd, 2) != 0) {
    printf("SHT31 write command failed\n");
    return false;
  }
  thread_sleep_for(20);

  char data[6] = {0};
  int ret = _i2c.read(SHT31_ADDR, data, 6);
  if (ret != 0) {
    printf("SHT31 read data failed (ret %d)\n", ret);
    return false;
  }

  printf("SHT31 raw: %02x %02x %02x %02x %02x %02x\n", data[0], data[1],
         data[2], data[3], data[4], data[5]);

  uint16_t st = (data[0] << 8) | data[1];
  uint16_t srh = (data[3] << 8) | data[4];

  temp = -45.0f + 175.0f * (float)st / 65535.0f;
  hum = 100.0f * (float)srh / 65535.0f;
  return true;
}

bool Sensors::read_bme280(float &temp, float &hum) {
  char cmd[1] = {0xF7};
  char data[8];
  if (_i2c.write(BME280_ADDR, cmd, 1) != 0)
    return false;
  if (_i2c.read(BME280_ADDR, data, 8) != 0)
    return false;

  int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
  int32_t adc_H = (data[6] << 8) | data[7];

  // Temperature compensation
  int32_t var1, var2;
  var1 = ((((adc_T >> 3) - ((int32_t)_bme280_comp.dig_T1 << 1))) *
          ((int32_t)_bme280_comp.dig_T2)) >>
         11;
  var2 = (((((adc_T >> 4) - ((int32_t)_bme280_comp.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)_bme280_comp.dig_T1))) >>
           12) *
          ((int32_t)_bme280_comp.dig_T3)) >>
         14;
  _bme280_comp.t_fine = var1 + var2;
  temp = (float)((_bme280_comp.t_fine * 5 + 128) >> 8) / 100.0f;

  // Humidity compensation
  int32_t v_x1_u32r;
  v_x1_u32r = (_bme280_comp.t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)_bme280_comp.dig_H4) << 20) -
                  (((int32_t)_bme280_comp.dig_H5) * v_x1_u32r)) +
                 ((int32_t)16384)) >>
                15) *
               (((((((v_x1_u32r * ((int32_t)_bme280_comp.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)_bme280_comp.dig_H3)) >> 11) +
                     ((int32_t)32768))) >>
                   10) +
                  ((int32_t)2097152)) *
                     ((int32_t)_bme280_comp.dig_H2) +
                 8192) >>
                14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)_bme280_comp.dig_H1)) >>
                            4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
  hum = (float)(v_x1_u32r >> 12) / 1024.0f;

  return true;
}

void Sensors::read_all(float &temp1, float &hum1, float &temp2, float &hum2) {
  if (!read_sht31(temp1, hum1)) {
    temp1 = -100.0f;
    hum1 = -1.0f;
  }
  if (!read_bme280(temp2, hum2)) {
    temp2 = -100.0f;
    hum2 = -1.0f;
  }
}

void Sensors::get_average_temp_and_humidity(float &avg_temp, float &avg_hum) {
  float t1, h1, t2, h2;
  read_all(t1, h1, t2, h2);

  int count = 0;
  avg_temp = 0.0f;
  avg_hum = 0.0f;

  if (t1 > -50.0f) {
    avg_temp += t1;
    avg_hum += h1;
    count++;
  }
  if (t2 > -50.0f) {
    avg_temp += t2;
    avg_hum += h2;
    count++;
  }

  if (count > 0) {
    avg_temp /= static_cast<float>(count);
    avg_hum /= static_cast<float>(count);
  }
}
