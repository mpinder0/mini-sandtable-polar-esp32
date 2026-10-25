#include "step_motion_mover.h"
#include <cmath>

StepMotionMover::StepMotionMover(AbstractMotorControl &motor_control, StepMotionPlanner &step_motion_planner)
    : motor_control(motor_control), planner(step_motion_planner), current_position({0, 0})
{
    // pass
}

StepMotionMover::~StepMotionMover()
{
    pattern.clear();
}

Coordinates StepMotionMover::get_current_position()
{
    return current_position;
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

bool StepMotionMover::reference()
{
    const int theta_steps_per_rotation = static_cast<int>(std::round(2.0f * static_cast<float>(PI) / AXIS_STEP_T));
    const int rho_steps_per_increment = static_cast<int>(std::round(REFERENCE_RHO_INCREMENT / AXIS_STEP_R));
    const int rho_max_steps = static_cast<int>(std::round((AXIS_MAX_R - AXIS_MIN_R) / AXIS_STEP_R));
    const float sensor_rho_position = AXIS_MAX_R - 4.0f; // The reference sensor is at the maximum rho position, minus and offset

    // Delay between steps to respect axis speed limits
    // note: theta using a separate (fatser) speed for referencing
    const unsigned int theta_step_delay_us = static_cast<unsigned int>(AXIS_STEP_T / AXIS_SPEED_T_REF * 1000000.0f);
    const unsigned int rho_step_delay_us = static_cast<unsigned int>(AXIS_STEP_R / AXIS_SPEED_R * 1000000.0f);

    // Rho position is unknown, so treat the starting position as rho = 0
    int rho_steps_done = 0;

    while (true)
    {
        printf("Referencing: full rotation at rho offset %.1f mm\n", rho_steps_done * AXIS_STEP_R);

        // Full rotation, checking the sensor before each step
        // (motor control handles the theta/rho coupling)
        for (int i = 0; i < theta_steps_per_rotation; ++i)
        {
            if (motor_control.get_reference_sensor_value())
            {
                printf("Reference sensor found\n");
                // Sensor position is theta zero, rho relative to the start of referencing
                current_position = {0.0f, sensor_rho_position};
                return true;
            }

            while (!motor_control.step(THETA))
            {
                delayMicroseconds(50);
            }
            delayMicroseconds(theta_step_delay_us);
        }

        if (rho_steps_done >= rho_max_steps)
        {
            // Reached the maximum rho extension without finding the reference. timeout.
            printf("Reference sensor not found after reaching maximum rho extension\n");
            break;
        }

        // Not found on this rotation, extend rho
        for (int i = 0; i < rho_steps_per_increment && rho_steps_done < rho_max_steps; ++i)
        {
            while (!motor_control.step(RHO))
            {
                delayMicroseconds(50);
            }
            rho_steps_done++;
            delayMicroseconds(rho_step_delay_us);
        }
    }

    printf("Reference sensor not found\n");
    return false;
}

void StepMotionMover::play_move(Move move)
{
    unsigned long start_ts = millis();
    unsigned long current_ts = start_ts;
    unsigned long end_ts = start_ts + move.moveTime;

    printf("Executing move: thetaSteps=%d, rhoSteps=%d, thetaDir=%s, rhoDir=%s, moveTime=%d ms\n",
           move.thetaSteps,
           move.rhoSteps,
           (move.thetaDir == FORWARD) ? "FORWARD" : "BACKWARD",
           (move.rhoDir == FORWARD) ? "FORWARD" : "BACKWARD",
           move.moveTime);

    // Calculate step intervals for each axis
    unsigned long theta_interval = (move.thetaSteps > 0) ? move.moveTime / move.thetaSteps : move.moveTime;
    unsigned long rho_interval = (move.rhoSteps > 0) ? move.moveTime / move.rhoSteps : move.moveTime;

    AxisStepper theta{THETA, move.thetaDir, move.thetaSteps, theta_interval, start_ts, 0};
    AxisStepper rho{RHO, move.rhoDir, move.rhoSteps, rho_interval, start_ts, 0};

    while (current_ts < end_ts)
    {
        current_ts = millis();

        // Step each axis if it's time
        step_axis_if_due(theta, current_ts);
        step_axis_if_due(rho, current_ts);

        // Small delay to prevent tight loop
        delayMicroseconds(MIN_STEP_DELAY_US);
    }
}

void StepMotionMover::step_axis_if_due(AxisStepper &ax, unsigned long current_ts)
{
    if (ax.steps_done < ax.total_steps && after_target_timestamp(current_ts, ax.next_ts))
    {
        // Time to step the axis
        // Retry on the next loop if the motor isn't ready
        if (!motor_control.step(ax.axis, ax.dir == BACKWARD))
        {
            return;
        }
        ax.steps_done++;
        ax.next_ts += ax.interval;

        // Update the current position based on the axis and direction
        if (ax.axis == THETA)
        {
            // Update current position for theta
            current_position.theta += (ax.dir == FORWARD) ? AXIS_STEP_T : -AXIS_STEP_T;
        }
        else if (ax.axis == RHO)
        {
            // Update current position for rho
            current_position.rho += (ax.dir == FORWARD) ? AXIS_STEP_R : -AXIS_STEP_R;
        }
    }
}