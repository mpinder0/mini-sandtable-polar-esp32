#include "step_motion_planner.h"
#include <cmath>

StepMotionPlanner::StepMotionPlanner()
{
    // No logging here: instances are usually globals, constructed before the Arduino core has
    // set the CPU clock. Serial output sent then is garbled, so log from setup() instead.
}

void StepMotionPlanner::print_speed_limits() const
{
    printf("Max speed - theta: %.5f rad/s, %.3f steps/s, %.4f ms/step\n", AXIS_SPEED_T, AXIS_SPEED_T / AXIS_STEP_T, 1000.0f / (AXIS_SPEED_T / AXIS_STEP_T));
    printf("Max speed - rho: %.5f mm/s, %.3f steps/s, %.4f ms/step\n", AXIS_SPEED_R, AXIS_SPEED_R / AXIS_STEP_R, 1000.0f / (AXIS_SPEED_R / AXIS_STEP_R));
    printf("Ramp - start rate theta: %.0f, rho: %.0f steps/s. accel theta: %.0f, rho: %.0f steps/s^2\n\n",
           AXIS_START_RATE_T, AXIS_START_RATE_R, AXIS_ACCEL_T, AXIS_ACCEL_R);
}

StepMotionPlanner::~StepMotionPlanner()
{
    // pass
}

void StepMotionPlanner::reset()
{
    prev_rate[THETA] = 0.0f;
    prev_rate[RHO] = 0.0f;
}

float StepMotionPlanner::allowed_rate(float prev_rate, bool same_direction, int steps, float start_rate, float accel)
{
    // Reversing, or starting from rest: the axis can only start at the start rate
    const float prev = same_direction ? std::fabs(prev_rate) : 0.0f;
    // Speed reachable over this move's steps at the acceleration limit (v^2 = u^2 + 2as),
    // and at most a start rate jump from the previous speed, as each move runs at one speed
    const float reachable = std::sqrt(prev * prev + 2.0f * accel * steps);
    return std::max(start_rate, std::min(reachable, prev + start_rate));
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
    float move_time = std::max(move_time_theta, move_time_rho);

    // Acceleration ramp: stretch the move so neither axis speeds up faster than it can follow.
    // Stretching slows both axes together, so the path is unchanged. Slowing down isn't limited,
    // stops and reversals are abrupt, but the restart after them is gentle.
    const float theta_prev = prev_rate[THETA];
    const float rho_prev = prev_rate[RHO];
    if (move.thetaSteps > 0)
    {
        const bool same_dir = (theta_prev >= 0.0f) == (steps_theta > 0);
        const float rate = allowed_rate(theta_prev, same_dir, move.thetaSteps, AXIS_START_RATE_T, AXIS_ACCEL_T);
        move_time = std::max(move_time, move.thetaSteps / rate);
    }
    if (move.rhoSteps > 0)
    {
        const bool same_dir = (rho_prev >= 0.0f) == (steps_rho > 0);
        const float rate = allowed_rate(rho_prev, same_dir, move.rhoSteps, AXIS_START_RATE_R, AXIS_ACCEL_R);
        move_time = std::max(move_time, move.rhoSteps / rate);
    }

    // Remember this move's speeds for the next. A move with no steps takes no time, so leaves
    // them unchanged. An axis with no steps in a moving move has stopped.
    if (move_time > 0.0f)
    {
        prev_rate[THETA] = steps_theta / move_time;
        prev_rate[RHO] = steps_rho / move_time;
    }

    move.moveTime = static_cast<unsigned long>(move_time * 1000000.0f); // Convert to microseconds

    return move;
}