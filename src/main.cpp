#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
// Motor driver selected by build flag, see platformio.ini and constants.h
#ifdef MOTOR_DRIVER_ULN2803
#include <motor_control_ULN2803.h>
using MotorControl = MotorControlULN2803;
#else
#include <motor_control_A4988.h>
using MotorControl = MotorControlA4988;
#endif
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

  motionMover.reference();

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