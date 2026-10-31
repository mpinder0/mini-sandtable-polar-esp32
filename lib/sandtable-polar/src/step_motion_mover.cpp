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

void StepMotionMover::set_theta_position(float theta)
{
    current_position.theta = theta;
}

void StepMotionMover::reset_theta_position()
{
    current_position.theta = std::fmod(current_position.theta, 2.0f * static_cast<float>(PI));
}

void StepMotionMover::set_pattern(std::vector<Coordinates> &new_pattern)
{
    pattern = new_pattern;
}

void StepMotionMover::play_pattern()
{
    // remove any theta rotations beyond full rotation. 2pi radians.
    reset_theta_position();

    // move to pattern start position
    Move mv = planner.get_move_steps(current_position, pattern[0]);
    play_move(mv);

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
    const float sensor_rho_position = AXIS_MAX_R - 5.0f; // The reference sensor is at the maximum rho position, minus and offset

    // Delay between steps to respect axis speed limits
    // note: theta using a separate (fatser) speed for referencing
    const unsigned int theta_step_delay_us = static_cast<unsigned int>(AXIS_STEP_T / AXIS_SPEED_T_REF * 1000000.0f);
    const unsigned int rho_step_delay_us = static_cast<unsigned int>(AXIS_STEP_R / AXIS_SPEED_R * 1000000.0f);

    // Rho position is unknown, so treat the starting position as rho = 0
    int rho_steps_done = 0;

    // 1. Seek: full rotations (motor control handles the theta/rho coupling), extending rho after each one
    printf("Referencing: full rotation at rho offset %.1f mm\n", rho_steps_done * AXIS_STEP_R);
    while (seek_sensor(THETA, false, true, theta_steps_per_rotation, theta_step_delay_us) < 0)
    {
        if (rho_steps_done >= rho_max_steps)
        {
            // Reached the maximum rho extension without finding the reference. timeout.
            printf("Reference sensor not found after reaching maximum rho extension\n");
            return false;
        }

        // Not found on this rotation, extend rho
        for (int i = 0; i < rho_steps_per_increment && rho_steps_done < rho_max_steps; ++i)
        {
            reference_step(RHO, false, rho_step_delay_us);
            rho_steps_done++;
        }
        printf("Referencing: full rotation at rho offset %.1f mm\n", rho_steps_done * AXIS_STEP_R);
    }
    printf("Reference sensor found, refining theta\n");

    // 2. Theta forward past the sensor until it turns off.
    // Theta step counts below are relative to where the sensor turned on going forward.
    const int theta_past_steps = seek_sensor(THETA, false, false, theta_steps_per_rotation, theta_step_delay_us);
    if (theta_past_steps < 0)
    {
        printf("Referencing failed: sensor did not turn off moving theta forward\n");
        return false;
    }

    // 3. Theta reverse until the sensor turns on again
    const int theta_back_steps = seek_sensor(THETA, true, true, theta_steps_per_rotation, theta_step_delay_us);
    if (theta_back_steps < 0)
    {
        printf("Referencing failed: sensor did not turn on moving theta in reverse\n");
        return false;
    }

    // 4. Move to the centre, between where the sensor turned on going forward (0) and in reverse
    const int theta_reverse_edge = theta_past_steps - theta_back_steps;
    const bool theta_to_centre_reverse = theta_reverse_edge > 0;
    const int theta_to_centre_steps = std::abs(theta_reverse_edge) / 2;
    for (int i = 0; i < theta_to_centre_steps; ++i)
    {
        reference_step(THETA, theta_to_centre_reverse, theta_step_delay_us);
    }
    printf("Theta centred on sensor, sensor width %d steps\n", std::abs(theta_reverse_edge));

    // 5. Rho retract until the sensor turns off
    const int rho_retract_steps = seek_sensor(RHO, true, false, rho_max_steps, rho_step_delay_us);
    if (rho_retract_steps < 0)
    {
        printf("Referencing failed: sensor did not turn off retracting rho\n");
        return false;
    }

    // 6. Rho forward until the sensor turns on again
    if (seek_sensor(RHO, false, true, rho_max_steps, rho_step_delay_us) < 0)
    {
        printf("Referencing failed: sensor did not turn on extending rho\n");
        return false;
    }

    // Sensor position is theta zero, rho at the known sensor position
    current_position = {0.0f, sensor_rho_position};
    printf("Reference complete\n");
    return true;
}

void StepMotionMover::reference_step(Axis ax, bool reverse, unsigned int step_delay_us)
{
    // Retry until the motor accepts the step
    while (!motor_control.step(ax, reverse))
    {
        delayMicroseconds(50);
    }
    delayMicroseconds(step_delay_us);
}

int StepMotionMover::seek_sensor(Axis ax, bool reverse, bool sensor_state, int max_steps, unsigned int step_delay_us)
{
    // Step until the reference sensor reads sensor_state, checking before each step
    for (int steps = 0; steps < max_steps; ++steps)
    {
        if (motor_control.get_reference_sensor_value() == sensor_state)
        {
            return steps;
        }
        reference_step(ax, reverse, step_delay_us);
    }

    return (motor_control.get_reference_sensor_value() == sensor_state) ? max_steps : -1;
}

void StepMotionMover::play_move(Move move)
{
    unsigned long start_ts = micros();
    unsigned long current_ts = start_ts;
    unsigned long end_ts = start_ts + move.moveTime;

    printf("Executing move: thetaSteps=%d, rhoSteps=%d, thetaDir=%s, rhoDir=%s, moveTime=%d us\n",
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
        current_ts = micros();

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