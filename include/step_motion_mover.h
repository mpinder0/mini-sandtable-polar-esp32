#ifndef MOVER_H
#define MOVER_H

#include <Arduino.h>
#include "motor_control.h"
#include "step_motion_planner.h"
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
    unsigned long theta_next_ts;
    unsigned long rho_next_ts;
    MotorControl &motor_control;
    StepMotionPlanner &planner;
    std::vector<Coordinates> pattern;

    bool at_target_timestamp(unsigned long current_time, unsigned long target_time);
};

#endif // MOVER_H