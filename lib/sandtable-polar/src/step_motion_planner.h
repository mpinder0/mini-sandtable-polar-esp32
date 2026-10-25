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

    Move get_move_steps(Coordinates start_pos, Coordinates end_pos);
};

#endif // PLANNER_H