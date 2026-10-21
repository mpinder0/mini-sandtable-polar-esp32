#ifndef PLANNER_H
#define PLANNER_H

#include <vector>
#include "position_helper.h"
#include "structs.h"

// Axis limits
// Theta not limited. in Radians
// Rho, in mm
const float AXIS_MIN_R = 0.0f;
const float AXIS_MAX_R = 71.5f;

const float microsteps_per_step = 4.0f; // 1/4 stepper driver
// Note: theta and rho axis are coupled, so for each theta step rho must also step (theta:rho - 8:1)
const int THETA_AXIS_GEAR_RATIO = 8; // 8:1 gear ratio

// step size - units
const float AXIS_STEP_T = deg_to_rad((1.8f / microsteps_per_step) / THETA_AXIS_GEAR_RATIO); // radians
// AXIS_STEP_R = (PI * 10mm) * (1.8deg / 360deg)
const float AXIS_STEP_R = 0.15707963f / microsteps_per_step; // mm

// speed - units per second
const float AXIS_SPEED_T = deg_to_rad(5.0f);
const float AXIS_SPEED_R = 5.0f;

class StepMotionPlanner
{
public:
    StepMotionPlanner();
    ~StepMotionPlanner();

    Move get_move_steps(Coordinates start_pos, Coordinates end_pos);
};

#endif // PLANNER_H