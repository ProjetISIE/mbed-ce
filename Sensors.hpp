#ifndef SENSORS_H
#define SENSORS_H

#include "mbed.h"

class Sensors {
public:
  Sensors(PinName sda, PinName scl);
  bool init();
  void read_all(float &temp1, float &hum1, float &temp2, float &hum2);
  void get_average_temp_and_humidity(float &avg_temp, float &avg_hum);

private:
  I2C _i2c;
  static constexpr int SHT31_ADDR = 0x40 << 1;
  static constexpr int BME280_ADDR = 0x76 << 1;

  bool init_sht31();
  bool init_bme280();
  bool read_sht31(float &temp, float &hum);
  bool read_bme280(float &temp, float &hum);

  // BME280 compensation parameters
  struct BME280_Comp {
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4, dig_H5;
    int8_t dig_H6;
    int32_t t_fine;
  } _bme280_comp;
};

#endif // SENSORS_H
