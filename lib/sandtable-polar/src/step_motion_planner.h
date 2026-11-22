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

    // Plan a move. The move time is stretched if needed so each axis's speed ramps up from
    // the previous move's, see the .cpp. Moves must be planned in the order they are played.
    Move get_move_steps(Coordinates start_pos, Coordinates end_pos);

    // Forget the previous move's speeds, call when the axes are at rest (e.g. pattern start)
    void reset();

private:
    // Signed step rate of each axis during the previous move, steps/s, indexed by Axis
    float prev_rate[2] = {0.0f, 0.0f};

    // Highest rate an axis may move at this move, given its previous rate and direction
    static float allowed_rate(float prev_rate, bool same_direction, int steps, float start_rate, float accel);
};

#endif // PLANNER_H