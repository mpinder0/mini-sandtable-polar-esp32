#ifndef PLANNER_H
#define PLANNER_H

#include <vector>
#include "position_helper.h"
#include "structs.h"
#include "constants.h"

class StepMotionPlanner
{
public:
    StepMotionPlanner();
    ~StepMotionPlanner();

    // Log the max axis speeds. Call from setup(), not a constructor (see constructor).
    void print_speed_limits() const;

    Move get_move_steps(Coordinates start_pos, Coordinates end_pos);
};

#endif // PLANNER_H