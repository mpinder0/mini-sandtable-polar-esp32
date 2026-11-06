#include "motor_control_A4988.h"

// Only built for the A4988 hardware (the default), see constants.h
#ifndef MOTOR_DRIVER_ULN2803

MotorControlA4988::MotorControlA4988()
    : theta_axis{THETA_STEP_PIN, THETA_DIR_PIN, AXIS_T_INVERT_DIR, "theta", 0},
      rho_axis{RHO_STEP_PIN, RHO_DIR_PIN, AXIS_R_INVERT_DIR, "rho", 0},
      enabled(false), theta_gear_step_counter(0)
{
    // Configure pins (skipped under UNIT_TEST)
#ifndef UNIT_TEST
    pinMode(REFERENCE_SENSOR_PIN, INPUT);
    pinMode(THETA_STEP_PIN, OUTPUT);
    pinMode(THETA_DIR_PIN, OUTPUT);
    pinMode(RHO_STEP_PIN, OUTPUT);
    pinMode(RHO_DIR_PIN, OUTPUT);
    pinMode(MOTORS_ENABLE_PIN, OUTPUT);

    // Initialize outputs to a safe state
    digitalWrite(THETA_STEP_PIN, LOW);
    digitalWrite(RHO_STEP_PIN, LOW);
    digitalWrite(THETA_DIR_PIN, LOW);
    digitalWrite(RHO_DIR_PIN, LOW);
#endif

    set_enable(false);

    printf("MotorControl initialized\n");
}

MotorControlA4988::~MotorControlA4988()
{
    // pass
}

void MotorControlA4988::set_enable(bool enable)
{
    enabled = enable;
#ifndef UNIT_TEST
    // Motor drivers are enabled with LOW (active low enable pin)
    digitalWrite(MOTORS_ENABLE_PIN, enable ? LOW : HIGH);
#endif
}

bool MotorControlA4988::is_enabled() const
{
    return enabled;
}

void MotorControlA4988::motors_release()
{
    set_enable(false);
}

bool MotorControlA4988::step(Axis ax, bool reverse)
{
    set_enable(true);

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

bool MotorControlA4988::theta_step(bool reverse)
{
    if (!axis_ready(theta_axis))
    {
        return false;
    }

    // Update the theta gear step counter and determine if a coupled rho step is needed
    const int8_t previous_counter = theta_gear_step_counter;
    int8_t rho_step_action = update_theta_counter(reverse ? BACKWARD : FORWARD);
    if (rho_step_action != 0 && !rho_step(rho_step_action > 0))
    {
        // Coupled rho step not possible yet, don't step theta either
        theta_gear_step_counter = previous_counter;
        return false;
    }

    pulse_axis(theta_axis, reverse);
    return true;
}

bool MotorControlA4988::rho_step(bool reverse)
{
    if (!axis_ready(rho_axis))
    {
        return false;
    }

    pulse_axis(rho_axis, reverse);
    return true;
}

bool MotorControlA4988::axis_ready(const AxisOutput &ax) const
{
    // Drivers enabled and minimum step delay elapsed since the last step
    return enabled && after_target_timestamp(micros(), ax.next_step_after);
}

void MotorControlA4988::pulse_axis(AxisOutput &ax, bool reverse)
{
    ax.next_step_after = micros() + MIN_STEP_DELAY_US;

    if (ax.invert_dir)
        reverse = !reverse;

#ifndef UNIT_TEST
    digitalWrite(ax.dir_pin, !reverse);

    // Pulse the step pin
    digitalWrite(ax.step_pin, HIGH);
    delayMicroseconds(STEP_PULSE_US);
    digitalWrite(ax.step_pin, LOW);
#else
    printf("%s step %s\n", ax.name, reverse ? "reverse" : "forward");
#endif
}

bool MotorControlA4988::get_reference_sensor_value() const
{
#ifndef UNIT_TEST
    // Sensor is active LOW
    return digitalRead(REFERENCE_SENSOR_PIN) == LOW;
#else
    // Under unit tests there is no hardware; return a safe default (not triggered)
    return false;
#endif
}

int8_t MotorControlA4988::update_theta_counter(Direction dir)
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

#endif // MOTOR_DRIVER_ULN2803
