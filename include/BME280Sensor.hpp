#ifndef BME280_SENSOR_HPP
#define BME280_SENSOR_HPP

#include "Sensor.hpp"
#include "mbed.h"
#include <cstdint>

class BME280Sensor : public Sensor {
public:
  BME280Sensor(I2C &i2c);
  bool init() override;
  bool read(float &temp, float &hum) override;
  const char *get_name() const override { return "BME280"; }

private:
  I2C &_i2c;
  static constexpr int ADDR = 0x77 << 1;

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
  } _comp;
};

#endif // BME280_SENSOR_HPP
