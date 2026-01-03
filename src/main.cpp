#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include <motor_control.h>
#include <structs.h>

void setup()
{
#ifndef UNIT_TEST
  Serial.begin(115200);
  Serial.println("MotorControl demo starting...");

  MotorControl mc;

  const unsigned long step_delay = 50; // ms
  const int steps_t = 100;
  const int steps_r = 100;

  Serial.println("Stepping Theta forward...");
  for (int i = 0; i < steps_t; ++i)
  {
    mc.step(THETA);
    delay(step_delay);
  }

  Serial.println("Stepping Theta backward...");
  for (int i = 0; i < steps_t; ++i)
  {
    mc.step(THETA, true);
    delay(step_delay);
  }

  mc.motors_release();
  Serial.println("Motors released (Theta).");

  Serial.println("Stepping Rho forward...");
  for (int i = 0; i < steps_r; ++i)
  {
    mc.step(RHO);
    delay(step_delay);
  }

  Serial.println("Stepping Rho backward...");
  for (int i = 0; i < steps_r; ++i)
  {
    mc.step(RHO, true);
    delay(step_delay);
  }

  mc.motors_release();
  Serial.println("Motors released (Rho). Done demo.");
#endif
}

void loop()
{
  // Nothing to do; demo runs in setup
  delay(1000);
}