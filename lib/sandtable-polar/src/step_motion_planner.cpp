#include "step_motion_planner.h"
#include <cmath>

StepMotionPlanner::StepMotionPlanner()
{
    // pass
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

    // Calculate the difference in position
    float delta_theta = end_pos.theta - start_pos.theta;
    float delta_rho = end_pos.rho - start_pos.rho;

    printf("------\n");
    printf("start_pos: %0.2f, %0.2f. end_pos: %0.2f, %0.2f. delta: %0.2f, %0.2f\n", start_pos.theta, start_pos.rho, end_pos.theta, end_pos.rho, delta_theta, delta_rho);

    // Calculate the number of steps for each axis
    int steps_theta = static_cast<int>(std::round(delta_theta / AXIS_STEP_T));
    int steps_rho = static_cast<int>(std::round(delta_rho / AXIS_STEP_R));

    printf("steps_theta: %d, steps_rho: %d\n", steps_theta, steps_rho);

    // Determine direction for each axis
    move.thetaDir = (steps_theta < 0) ? BACKWARD : FORWARD;
    move.rhoDir = (steps_rho < 0) ? BACKWARD : FORWARD;

    // Remove the sign from the step counts if negative
    move.thetaSteps = std::abs(steps_theta);
    move.rhoSteps = std::abs(steps_rho);

    // Calculate move time for each axis (time = distance / speed)
    float move_time_theta = std::abs(delta_theta) / AXIS_SPEED_T;
    float move_time_rho = std::abs(delta_rho) / AXIS_SPEED_R;

    printf("delta_theta: %.2f, delta_rho: %.2f, speed_theta: %.2f rad/s, speed_rho: %.2f mm/s\n", delta_theta, delta_rho, AXIS_SPEED_T, AXIS_SPEED_R);
    printf("move_time_theta: %.2f s, move_time_rho: %.2f s\n", move_time_theta, move_time_rho);

    // Use the maximum time required for either axis (for slowest axis)
    move.moveTime = static_cast<unsigned long>(std::max(move_time_theta, move_time_rho) * 1000000.0f); // Convert to microseconds

    printf("us between steps - theta: %.2f, rho: %.2f\n", (move.thetaSteps > 0) ? (float)move.moveTime / (float)move.thetaSteps : move.moveTime, (move.rhoSteps > 0) ? (float)move.moveTime / (float)move.rhoSteps : move.moveTime);

    return move;
}