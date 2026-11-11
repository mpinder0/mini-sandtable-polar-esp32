#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include "abstract_motor_control.h"
#include "time_helper.h"
#include "constants.h"

void AbstractMotorControl::set_enable(bool enable)
{
    enabled = enable;
    write_enable(enable);
}

bool AbstractMotorControl::is_enabled() const
{
    return enabled;
}

void AbstractMotorControl::motors_release()
{
    set_enable(false);
}

bool AbstractMotorControl::step(Axis ax, bool reverse)
{
    if (!enabled)
    {
        set_enable(true);
    }

    if (ax == THETA)
    {
        return theta_step(reverse);
    }
    else if (ax == RHO)
    {
        return rho_step(reverse);
    }
    return false;
}

bool AbstractMotorControl::theta_step(bool reverse)
{
    if (!axis_ready(THETA))
    {
        return false;
    }

    // Update the theta gear step counter and determine if a coupled rho step is needed
    const int8_t previous_counter = theta_gear_step_counter;
    int8_t rho_step_action = update_theta_counter(reverse ? BACKWARD : FORWARD);
    if (rho_step_action != 0)
    {
        bool rho_stepped = rho_step(rho_step_action > 0 ? FORWARD : BACKWARD);
        if (!rho_stepped)
        {
            // Coupled rho step failed, revert theta counter and do not step theta
            theta_gear_step_counter = previous_counter;
            return false;
        }
    }

    pulse_axis(THETA, reverse);
    return true;
}

bool AbstractMotorControl::rho_step(bool reverse)
{
    if (!axis_ready(RHO))
    {
        return false;
    }

    pulse_axis(RHO, reverse);
    return true;
}

bool AbstractMotorControl::axis_ready(Axis ax) const
{
    // Motors enabled and minimum step delay elapsed since the last step
    return enabled && after_target_timestamp(micros(), next_step_after[ax]);
}

void AbstractMotorControl::pulse_axis(Axis ax, bool reverse)
{
    next_step_after[ax] = micros() + MIN_STEP_DELAY_US;
    write_step(ax, reverse);
}

int8_t AbstractMotorControl::update_theta_counter(Direction dir)
{
    if (dir == FORWARD)
    {
        theta_gear_step_counter++;
        if (theta_gear_step_counter >= THETA_AXIS_GEAR_RATIO)
        {
            theta_gear_step_counter = 0;
            return 1; // One full step of the coupled axis
        }
    }
    else if (dir == BACKWARD)
    {
        theta_gear_step_counter--;
        if (theta_gear_step_counter < 0)
        {
            theta_gear_step_counter = THETA_AXIS_GEAR_RATIO - 1;
            return -1; // One full step of the coupled axis
        }
    }
    return 0; // No full step yet
}
