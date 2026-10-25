#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include <motor_control.h>
#include <step_motion_planner.h>
#include <step_motion_mover.h>
#include <structs.h>
#include <Pattern/pattern_spiral.cpp>
#include <Pattern/pattern_radial_sweep.cpp>

MotorControl motorControl;
StepMotionPlanner motionPlanner;
StepMotionMover motionMover(motorControl, motionPlanner);
std::vector<Coordinates> patternSpiral;
std::vector<Coordinates> patternRadial;

void setup()
{
#ifndef UNIT_TEST
  patternSpiral = PatternSpiral().get_pattern();
  patternRadial = PatternRadialSweep().get_pattern();

  for (int i = 0; i < patternRadial.size(); ++i)
  {
    // print radial sweep pattern coordinates
    printf("PatternRadialSweep[%d]: theta=%.2f, rho=%.2f\n", i, patternRadial[i].theta, patternRadial[i].rho);
  }

  motionMover.reference();

  /*
  // mote to 0,0
  Move mv = motionPlanner.get_move_steps(motionMover.get_current_position(), {0, 0});
  motionMover.play_move(mv);
  printf("Current position after move to 0,0: theta=%.2f, rho=%.2f\n", motionMover.get_current_position().theta, motionMover.get_current_position().rho);
  motorControl.motors_release();
  */

  motionMover.set_pattern(patternRadial);
  motionMover.play_pattern();

  motionMover.set_pattern(patternSpiral);
  motionMover.play_pattern();

  motorControl.motors_release();
#endif
}

void loop()
{
  delay(1000);
}