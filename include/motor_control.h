#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include "Structs.h"  

class MotorControl {
public:
  MotorControl();
  ~MotorControl();
 
  void motors_release();
  void step(Axis ax, bool reverse = false);
  bool get_reference_sensor_value() const;

private:
  unsigned long theta_timestamp_ms;
  unsigned long rho_timestamp_ms;
  bool enabled;

  // Pin definitions
  static constexpr int REFERENCE_SENSOR_PIN = 5; // sensor input (active LOW)
  static constexpr int THETA_STEP_PIN = 2;
  static constexpr int THETA_DIR_PIN = 4;
  static constexpr int RHO_STEP_PIN = 16;
  static constexpr int RHO_DIR_PIN = 17;
  static constexpr int MOTORS_ENABLE_PIN = 15; // active LOW

  // Timing constants
  // Minimum delay between steps in milliseconds
  static constexpr unsigned long MIN_STEP_DELAY_MS = 5;
  // Step pulse width in microseconds (1 ms = 1000 us)
  static constexpr unsigned int STEP_PULSE_US = 1000;

  // Direction inversion flags (set to true if wiring inverts intended direction)
  static constexpr bool AXIS_T_INVERT_DIR = false;
  static constexpr bool AXIS_R_INVERT_DIR = false;

  void theta_step(bool reverse = false);
  void rho_step(bool reverse = false);
  void set_enable(bool enable);
};

#endif // MOTOR_CONTROL_H
