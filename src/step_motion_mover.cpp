#include "step_motion_mover.h"
#include "step_motion_planner.h"
#include <cmath>

StepMotionMover::StepMotionMover(MotorControl &motor_control, StepMotionPlanner &step_motion_planner)
    : theta_next_ts(0), rho_next_ts(0), motor_control(motor_control), planner(step_motion_planner)
{
    // pass
}

StepMotionMover::~StepMotionMover()
{
    pattern.clear();
}

void StepMotionMover::set_pattern(std::vector<Coordinates> &new_pattern)
{
    pattern = new_pattern;
}

void StepMotionMover::play_pattern()
{
    // Loop while moves available in pattern
    for (size_t i = 0; i < pattern.size() - 1; ++i)
    {
        // Get move from current to next position
        Move mv = planner.get_move_steps(pattern[i], pattern[i + 1]);
        // Execute the move
        play_move(mv);
    }
}

bool StepMotionMover::at_target_timestamp(unsigned long current_time, unsigned long target_time)
{
    // Handle wrap-around by using time delta, unsigned arithmetic
    const auto delta = target_time - current_time;
    // Cast as signed to test for negative (overflowed)
    return static_cast<int32_t>(delta) <= 0;
}

void StepMotionMover::play_move(Move move)
{
    unsigned long start_ts = millis();
    unsigned long current_ts = start_ts;
    unsigned long end_ts = start_ts + move.moveTime;

    // Calculate step intervals for each axis
    unsigned long theta_interval = (move.thetaSteps > 0) ? move.moveTime / move.thetaSteps : move.moveTime;
    unsigned long rho_interval = (move.rhoSteps > 0) ? move.moveTime / move.rhoSteps : move.moveTime;

    unsigned long next_theta_ts = start_ts;
    unsigned long next_rho_ts = start_ts;

    int theta_steps_done = 0;
    int rho_steps_done = 0;

    while (current_ts < end_ts)
    {
        current_ts = millis();

        // Step theta axis if it's time
        if (theta_steps_done < move.thetaSteps && at_target_timestamp(current_ts, next_theta_ts))
        {
            motor_control.step(THETA, move.thetaDir == BACKWARD);
            theta_steps_done++;
            next_theta_ts += theta_interval;
        }

        // Step rho axis if it's time
        if (rho_steps_done < move.rhoSteps && at_target_timestamp(current_ts, next_rho_ts))
        {
            motor_control.step(RHO, move.rhoDir == BACKWARD);
            rho_steps_done++;
            next_rho_ts += rho_interval;
        }

        // Small delay to prevent tight loop
        delayMicroseconds(500); // 0.5 ms
    }
}