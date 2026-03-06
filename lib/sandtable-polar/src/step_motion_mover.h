#ifndef MOVER_H
#define MOVER_H

#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include "motor_control.h"
#include "step_motion_planner.h"
#include "time_helper.h"
#include <vector>

class StepMotionMover
{
public:
    StepMotionMover(MotorControl &motor_control, StepMotionPlanner &motion_planner);
    ~StepMotionMover();

    void set_pattern(std::vector<Coordinates> &new_pattern);
    void play_pattern();
    void play_move(Move move);

private:
    MotorControl &motor_control;
    StepMotionPlanner &planner;
    std::vector<Coordinates> pattern;
};

#endif // MOVER_H