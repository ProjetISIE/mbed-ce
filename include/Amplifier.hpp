#ifndef AMPLIFIER_HPP
#define AMPLIFIER_HPP

class Amplifier {
public:
  virtual ~Amplifier() = default;
  virtual void on() = 0;
  virtual void off() = 0;
  virtual void log_status(float freq, float amp, float mod_rate) = 0;
};

#endif // AMPLIFIER_HPP
