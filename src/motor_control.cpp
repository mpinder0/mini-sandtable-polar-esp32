#include "motor_control.h"
#include <Arduino.h>

MotorControl::MotorControl()
  : theta_timestamp_ms(0), rho_timestamp_ms(0), enabled(true)
{
  // Configure pins
  pinMode(REFERENCE_SENSOR_PIN, INPUT_PULLUP);
  pinMode(THETA_STEP_PIN, OUTPUT);
  pinMode(THETA_DIR_PIN, OUTPUT);
  pinMode(RHO_STEP_PIN, OUTPUT);
  pinMode(RHO_DIR_PIN, OUTPUT);
  pinMode(MOTORS_ENABLE_PIN, OUTPUT);

  // Initialize outputs to a safe state
  digitalWrite(THETA_STEP_PIN, LOW);
  digitalWrite(RHO_STEP_PIN, LOW);
  digitalWrite(THETA_DIR_PIN, LOW);
  digitalWrite(RHO_DIR_PIN, LOW);

  // Set timestamps so first step can occur immediately
  unsigned long now = millis();
  theta_timestamp_ms = now - MIN_STEP_DELAY_MS;
  rho_timestamp_ms = now - MIN_STEP_DELAY_MS;

  set_enable(true);
}

MotorControl::~MotorControl() {}

void MotorControl::set_enable(bool enable)
{
  enabled = enable;
  // Motor drivers are enabled with LOW (active low enable pin)
  digitalWrite(MOTORS_ENABLE_PIN, enable ? LOW : HIGH);
}

void MotorControl::motors_release()
{
  set_enable(false);
}

void MotorControl::step(Axis ax, bool reverse)
{
  set_enable(true);

  if (ax == AXIS_THETA) {
    theta_step(reverse);
  } else if (ax == AXIS_RHO) {
    rho_step(reverse);
  }
}

void MotorControl::theta_step(bool reverse)
{
  if (!enabled) {
    // Drivers are disabled; nothing to do
    return;
  }

  // Wait for minimum step delay
  while (millis() - theta_timestamp_ms < MIN_STEP_DELAY_MS) {
    delay(1);
  }

  if (AXIS_T_INVERT_DIR) reverse = !reverse;

  digitalWrite(THETA_DIR_PIN, !reverse);

  // Pulse the step pin
  digitalWrite(THETA_STEP_PIN, HIGH);
  delayMicroseconds(STEP_PULSE_US);
  digitalWrite(THETA_STEP_PIN, LOW);

  theta_timestamp_ms = millis();
}

void MotorControl::rho_step(bool reverse)
{
  if (!enabled) {
    return;
  }

  while (millis() - rho_timestamp_ms < MIN_STEP_DELAY_MS) {
    delay(1);
  }

  if (AXIS_R_INVERT_DIR) reverse = !reverse;

  digitalWrite(RHO_DIR_PIN, !reverse);

  digitalWrite(RHO_STEP_PIN, HIGH);
  delayMicroseconds(STEP_PULSE_US);
  digitalWrite(RHO_STEP_PIN, LOW);

  rho_timestamp_ms = millis();
}

bool MotorControl::get_reference_sensor_value() const
{
  // Sensor is active LOW
  return digitalRead(REFERENCE_SENSOR_PIN) == LOW;
}
