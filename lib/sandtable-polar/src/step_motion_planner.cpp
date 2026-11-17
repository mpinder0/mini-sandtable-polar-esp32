#include "step_motion_planner.h"
#include <cmath>

StepMotionPlanner::StepMotionPlanner()
{
    // No logging here: instances are usually globals, constructed before the Arduino core has
    // set the CPU clock. Serial output sent then is garbled, so log from setup() instead.
}

void StepMotionPlanner::print_speed_limits() const
{
    printf("Max speed - theta: %.5f rad/s, %.3f steps/s, %.2f ms/step\n", AXIS_SPEED_T, AXIS_SPEED_T / AXIS_STEP_T, 1000.0f / (AXIS_SPEED_T / AXIS_STEP_T));
    printf("Max speed - rho: %.5f mm/s, %.3f steps/s, %.2f ms/step\n\n", AXIS_SPEED_R, AXIS_SPEED_R / AXIS_STEP_R, 1000.0f / (AXIS_SPEED_R / AXIS_STEP_R));
}

StepMotionPlanner::~StepMotionPlanner()
{
    // pass
}

Move StepMotionPlanner::get_move_steps(Coordinates start_pos, Coordinates end_pos)
{
    Move move;

    move.targetPosition = end_pos;

    // Steps are the difference between each position snapped to the step grid, rather than the
    // rounded difference. Rounding then can't accumulate across moves: the steps for a sequence
    // of moves always sum to the grid distance from the first point to the last.
    int steps_theta = position_to_steps(end_pos.theta, AXIS_STEP_T) - position_to_steps(start_pos.theta, AXIS_STEP_T);
    int steps_rho = position_to_steps(end_pos.rho, AXIS_STEP_R) - position_to_steps(start_pos.rho, AXIS_STEP_R);

    // Determine direction for each axis
    move.thetaDir = (steps_theta < 0) ? BACKWARD : FORWARD;
    move.rhoDir = (steps_rho < 0) ? BACKWARD : FORWARD;

    // Remove the sign from the step counts if negative
    move.thetaSteps = std::abs(steps_theta);
    move.rhoSteps = std::abs(steps_rho);

    // Calculate move time for each axis (time = distance / speed), from the steps actually taken
    float move_time_theta = move.thetaSteps * AXIS_STEP_T / AXIS_SPEED_T;
    float move_time_rho = move.rhoSteps * AXIS_STEP_R / AXIS_SPEED_R;

    // Use the maximum time required for either axis (for slowest axis)
    move.moveTime = static_cast<unsigned long>(std::max(move_time_theta, move_time_rho) * 1000000.0f); // Convert to microseconds

    // printf("us between steps - theta: %.2f, rho: %.2f\n", (move.thetaSteps > 0) ? (float)move.moveTime / (float)move.thetaSteps : move.moveTime, (move.rhoSteps > 0) ? (float)move.moveTime / (float)move.rhoSteps : move.moveTime);

    return move;
}