#include "step_motion_mover.h"
#include <cmath>

StepMotionMover::StepMotionMover(AbstractMotorControl &motor_control, StepMotionPlanner &step_motion_planner)
    : motor_control(motor_control), planner(step_motion_planner)
{
    // pass
}

StepMotionMover::~StepMotionMover()
{
    pattern = nullptr;
}

Coordinates StepMotionMover::get_current_position()
{
    return {theta_steps_pos * AXIS_STEP_T, rho_steps_pos * AXIS_STEP_R};
}

void StepMotionMover::set_position(Coordinates position)
{
    theta_steps_pos = position_to_steps(position.theta, AXIS_STEP_T);
    rho_steps_pos = position_to_steps(position.rho, AXIS_STEP_R);
}

void StepMotionMover::set_theta_position(float theta)
{
    theta_steps_pos = position_to_steps(theta, AXIS_STEP_T);
}

void StepMotionMover::reset_theta_position()
{
    // Remove whole rotations, keeping the sign like fmod. Exact, as it's in whole steps.
    static const int32_t theta_steps_per_rotation = position_to_steps(2.0f * static_cast<float>(PI), AXIS_STEP_T);
    theta_steps_pos %= theta_steps_per_rotation;
}

void StepMotionMover::set_pattern(const std::vector<Coordinates> &new_pattern)
{
    pattern = &new_pattern;
}

void StepMotionMover::play_pattern()
{
    if (pattern == nullptr || pattern->empty())
        return;
    const std::vector<Coordinates> &points = *pattern;

    // remove any theta rotations beyond full rotation. 2pi radians.
    reset_theta_position();

    // move to pattern start position
    const Coordinates start_position = get_current_position();
    Move mv = planner.get_move_steps(start_position, points[0]);

    printf("*** Starting new pattern: Moving from (%.3f, %.3f) to pattern start (%.3f, %.3f) ***\n", start_position.theta, start_position.rho, points[0].theta, points[0].rho);

    play_move(mv);

    printf("*** Pattern start reached ***\n");

    // Loop while moves available in pattern
    for (size_t i = 1; i < points.size(); ++i)
    {
        // Plan from where the motors actually are, not the previous pattern point, so
        // per move step rounding is corrected on the next move instead of accumulating
        Move mv = planner.get_move_steps(get_current_position(), points[i]);
        // Execute the move
        play_move(mv);
    }
}

bool StepMotionMover::reference()
{
    const int theta_steps_per_rotation = static_cast<int>(std::round(2.0f * static_cast<float>(PI) / AXIS_STEP_T));
    const int rho_steps_per_increment = static_cast<int>(std::round(REFERENCE_RHO_INCREMENT / AXIS_STEP_R));
    const int rho_max_steps = static_cast<int>(std::round((AXIS_MAX_R - AXIS_MIN_R) / AXIS_STEP_R));
    const float sensor_rho_position = AXIS_MAX_R - 6.0f; // The reference sensor is at the maximum rho position, minus and offset

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
    set_position({0.0f, sensor_rho_position});
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
#ifdef DEBUG_MOVE_LOG
    printf("Executing move - position:(%.3f, %.3f), steps:(%d, %d), direction:(%s,%s), moveTime=%.3f s\n",
           move.targetPosition.theta,
           move.targetPosition.rho,
           move.thetaSteps,
           move.rhoSteps,
           (move.thetaDir == FORWARD) ? "FORWARD" : "BACKWARD",
           (move.rhoDir == FORWARD) ? "FORWARD" : "BACKWARD",
           move.moveTime / 1000000.0f);
#endif

    // Calculate step intervals for each axis
    const uint32_t theta_interval = (move.thetaSteps > 0) ? move.moveTime / move.thetaSteps : move.moveTime;
    const uint32_t rho_interval = (move.rhoSteps > 0) ? move.moveTime / move.rhoSteps : move.moveTime;

    // Start timing after the (debug) log line, so time spent blocked on serial output
    // is not taken out of the move
    const uint32_t start_ts = static_cast<uint32_t>(micros());

    AxisStepper theta{THETA, move.thetaDir, move.thetaSteps, theta_interval, start_ts, 0};
    AxisStepper rho{RHO, move.rhoDir, move.rhoSteps, rho_interval, start_ts, 0};

    // Run until the move time has elapsed AND every step has been taken, so steps
    // delayed by the minimum step delay or the theta/rho coupling are not dropped.
    // Elapsed time is an unsigned difference, so this is correct across a micros() wrap
    // (a plain current_ts < end_ts comparison could stall for ~71.6 minutes or skip the move).
    while (true)
    {
        const uint32_t current_ts = static_cast<uint32_t>(micros());
        const bool time_done = elapsed_since(current_ts, start_ts) >= move.moveTime;
        const bool steps_done = theta.steps_done >= theta.total_steps && rho.steps_done >= rho.total_steps;
        if (time_done && steps_done)
            break;

        // Step each axis if it's time
        step_axis_if_due(theta, current_ts);
        step_axis_if_due(rho, current_ts);

        // Short poll delay, so steps aren't held back by up to a whole min step delay
        delayMicroseconds(MOVE_LOOP_DELAY_US);
    }
}

void StepMotionMover::step_axis_if_due(AxisStepper &ax, uint32_t current_ts)
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

        // Update the step position based on the axis and direction
        const int32_t step = (ax.dir == FORWARD) ? 1 : -1;
        if (ax.axis == THETA)
            theta_steps_pos += step;
        else if (ax.axis == RHO)
            rho_steps_pos += step;
    }
}