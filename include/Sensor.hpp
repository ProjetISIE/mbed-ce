#ifndef SENSOR_HPP
#define SENSOR_HPP

#include "mbed.h"

class Sensor {
public:
  virtual ~Sensor() = default;
  virtual bool init() = 0;
  virtual bool read(float &temp, float &hum) = 0;
  virtual const char *get_name() const = 0;
};

#endif // SENSOR_HPP
