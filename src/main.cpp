#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include <motor_control.h>
#include <step_motion_planner.h>
#include <step_motion_mover.h>
#include <structs.h>

void setup()
{
#ifndef UNIT_TEST
  MotorControl motorControl;
  StepMotionPlanner motionPlanner;
  StepMotionMover motionMover(motorControl, motionPlanner);

  std::vector<Coordinates> pattern = {
      {0, 0},
      {1.570796f, 0},
      {1.570796f, 20},
      {1.570796f, 0}};

  motionMover.set_pattern(pattern);
  motionMover.play_pattern();
  motorControl.motors_release();

  /*
  motorControl.set_enable(true);

  for (int i = 0; i < 100; ++i)
  {
    motorControl.step(THETA, false);
    delay(50);
  }
  for (int i = 0; i < 100; ++i)
  {
    motorControl.step(THETA, true);
    delay(50);
  }
  delay(2000);

  for (int i = 0; i < 100; ++i)
  {
    motorControl.step(RHO, false);
    delay(50);
  }
  for (int i = 0; i < 100; ++i)
  {
    motorControl.step(RHO, true);
    delay(50);
  }
  motorControl.motors_release();
  */
#endif
}

void loop()
{
  delay(1000);
}