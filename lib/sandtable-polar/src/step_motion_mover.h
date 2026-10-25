#ifndef MOVER_H
#define MOVER_H

#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include "abstract_motor_control.h"
#include "step_motion_planner.h"
#include "time_helper.h"
#include <vector>

class StepMotionMover
{
public:
    StepMotionMover(AbstractMotorControl &motor_control, StepMotionPlanner &motion_planner);
    ~StepMotionMover();

    Coordinates get_current_position();
    void set_pattern(std::vector<Coordinates> &new_pattern);
    void play_pattern();
    void play_move(Move move);
    // Rotate theta looking for the reference sensor, extending rho after each full rotation.
    // Returns true if the reference was found.
    bool reference();

private:
    // Rho extension between each full reference rotation, in mm
    static constexpr float REFERENCE_RHO_INCREMENT = 2.0f;

    AbstractMotorControl &motor_control;
    StepMotionPlanner &planner;
    std::vector<Coordinates> pattern;
    Coordinates current_position = {0, 0};

    void step_axis_if_due(AxisStepper &ax, unsigned long current_ts);
};

#endif // MOVER_H