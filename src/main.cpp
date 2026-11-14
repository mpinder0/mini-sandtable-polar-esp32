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
#include <Pattern/pattern_file.h>
#ifndef UNIT_TEST
#include <LittleFS.h>
#endif

MotorControl motorControl;
StepMotionPlanner motionPlanner;
StepMotionMover motionMover(motorControl, motionPlanner);
std::vector<Coordinates> patternSpiral;
std::vector<Coordinates> patternRadial;
std::vector<Coordinates> patternFromFile_Star;
std::vector<Coordinates> patternFromFile_10SidePoly;

// .thr pattern on LittleFS, uploaded from data/ with: pio run -t uploadfs
static const char *PATTERN_FILE_PATH_STAR = "/patterns/star.thr";
static const char *PATTERN_FILE_PATH_10_SIDED_POLYGON = "/patterns/10_sided_polygon.thr";

void setup()
{
#ifndef UNIT_TEST
  patternSpiral = PatternSpiral().get_pattern();
  // patternRadial = PatternRadialSweep().get_pattern();

  if (LittleFS.begin())
  {
    PatternFile patternFileStar(LittleFS, PATTERN_FILE_PATH_STAR);
    if (patternFileStar.is_valid())
      patternFromFile_Star = patternFileStar.take_pattern(); // move, no copy

    PatternFile patternFile10SidePoly(LittleFS, PATTERN_FILE_PATH_10_SIDED_POLYGON);
    if (patternFile10SidePoly.is_valid())
      patternFromFile_10SidePoly = patternFile10SidePoly.take_pattern(); // move, no copy
  }
  else
  {
    printf("LittleFS mount failed, has the filesystem been uploaded? (pio run -t uploadfs)\n");
  }

  motionMover.reference();

  if (!patternFromFile_Star.empty())
  {
    motionMover.set_pattern(patternFromFile_Star);
    motionMover.play_pattern();
  }

  // motionMover.set_pattern(patternRadial);
  // motionMover.play_pattern();

  motionMover.set_pattern(patternSpiral);
  motionMover.play_pattern();

  if (!patternFromFile_10SidePoly.empty())
  {
    motionMover.set_pattern(patternFromFile_10SidePoly);
    motionMover.play_pattern();
  }

  motorControl.motors_release();
#endif
}

void loop()
{
  delay(1000);
}