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
// Kept resident, it is played between every file pattern
std::vector<Coordinates> patternSpiral;

// .thr patterns on LittleFS, uploaded from data/ with: pio run -t uploadfs
static const char *PATTERN_DIR = "/patterns";

void setup()
{
#ifndef UNIT_TEST
  patternSpiral = PatternSpiral().get_pattern();

  motionMover.reference();

  if (LittleFS.begin())
  {
    const std::vector<String> paths = PatternFile::list(LittleFS, PATTERN_DIR);
    printf("Found %u pattern file(s) in %s\n", static_cast<unsigned>(paths.size()), PATTERN_DIR);

    bool played_any = false;
    for (const String &path : paths)
    {
      // Scoped to this iteration, so the points are freed before the next file is loaded
      std::vector<Coordinates> pattern = PatternFile::load(LittleFS, path.c_str());
      if (pattern.empty())
        continue;

      // Spiral between file patterns, to clear the previous one before the next is drawn
      if (played_any)
      {
        motionMover.set_pattern(patternSpiral);
        motionMover.play_pattern();
      }

      printf("Playing %s (%u points), free heap %u bytes\n", path.c_str(),
             static_cast<unsigned>(pattern.size()), static_cast<unsigned>(ESP.getFreeHeap()));
      motionMover.set_pattern(pattern);
      motionMover.play_pattern();
      // The mover holds a reference, point it back at the resident spiral before pattern is freed
      motionMover.set_pattern(patternSpiral);
      played_any = true;
    }
  }
  else
  {
    printf("LittleFS mount failed, has the filesystem been uploaded? (pio run -t uploadfs)\n");
    motionMover.set_pattern(patternSpiral);
    motionMover.play_pattern();
  }

  motorControl.motors_release();
#endif
}

void loop()
{
  delay(1000);
}
