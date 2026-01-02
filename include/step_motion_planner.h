#ifndef PLANNER_H
#define PLANNER_H

#include <vector>
#include "position_helper.h"
#include "structs.h"

// Axis limits
// Theta not limited. in Radians
// Rho, in mm
const float AXIS_MAX_R = 0.0f;
const float AXIS_MAX_R = 71.5f;
// step size - units
const float AXIS_STEP_T = deg_to_rad(1.8f); // radians
// AXIS_STEP_R = (PI * 10mm) * (1.8deg / 360deg)
const float AXIS_STEP_R = 0.15707963f; // mm

// note: theta and rho axis are coupled, so for each theta step rho must also step (theta:rho - 8:1)
const int AXIS_COUPLING_RATIO = 8; // 8:1 gear ratio

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