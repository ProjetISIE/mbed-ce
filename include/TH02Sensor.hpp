#ifndef TH02_SENSOR_HPP
#define TH02_SENSOR_HPP

#include "Sensor.hpp"
#include "mbed.h"

class TH02Sensor : public Sensor {
public:
  TH02Sensor(I2C &i2c);
  bool init() override;
  bool read(float &temp, float &hum) override;
  const char *get_name() const override { return "TH02"; }

private:
  I2C &_i2c;
  static constexpr int ADDR = 0x40 << 1;
};

#endif // TH02_SENSOR_HPP
