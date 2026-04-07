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
      {0.174533, 0},
      {0.174533, 10},
      {0.174533, 10},
      {0.174533, 0}};

  motionMover.set_pattern(pattern);
  motionMover.play_pattern();
#endif
}

void loop()
{
  delay(1000);
}