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

  motionMover.reference();

  // mote to 0,0
  Move mv = motionPlanner.get_move_steps(motionMover.get_current_position(), {0, 0});
  motionMover.play_move(mv);
  printf("Current position after move to 0,0: theta=%.2f, rho=%.2f\n", motionMover.get_current_position().theta, motionMover.get_current_position().rho);

  motorControl.motors_release();

  std::vector<Coordinates> pattern = {
      {0, 0},
      {1.570796f, 0},
      {1.570796f, 30},
      {1.570796f, 0},
      {1.570796f, AXIS_MAX_R}};

  motionMover.set_pattern(pattern);
  motionMover.play_pattern();
  motorControl.motors_release();
#endif
}

void loop()
{
  delay(1000);
}