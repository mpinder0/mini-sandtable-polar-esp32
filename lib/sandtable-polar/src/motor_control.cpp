#include "motor_control.h"

MotorControl::MotorControl()
    : theta_next_step_after(0), rho_next_step_after(0), enabled(false), theta_gear_step_counter(0)
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

MotorControl::~MotorControl()
{
    // pass
}

void MotorControl::set_enable(bool enable)
{
    enabled = enable;
#ifndef UNIT_TEST
    // Motor drivers are enabled with LOW (active low enable pin)
    digitalWrite(MOTORS_ENABLE_PIN, enable ? LOW : HIGH);
#endif
}

bool MotorControl::is_enabled() const
{
    return enabled;
}

void MotorControl::motors_release()
{
    set_enable(false);
}

void MotorControl::step(Axis ax, bool reverse)
{
    set_enable(true);

    if (ax == THETA)
    {
        theta_step(reverse);
    }
    else if (ax == RHO)
    {
        rho_step(reverse);
    }
}

void MotorControl::theta_step(bool reverse)
{
    if (!enabled)
    {
        // Drivers are disabled; nothing to do
        return;
    }

    // Wait for minimum step delay
    while (!after_target_timestamp(micros(), theta_next_step_after))
    {
        delayMicroseconds(50);
    }
    theta_next_step_after = micros() + MIN_STEP_DELAY_US;

    if (AXIS_T_INVERT_DIR)
        reverse = !reverse;

    // Update the theta gear step counter and determine if a coupled rho step is needed
    int8_t rho_step_action = update_theta_counter(reverse ? BACKWARD : FORWARD);
    if (rho_step_action != 0)
    {
        bool rho_dir = rho_step_action > 0;
        rho_step(rho_dir);
    }

#ifndef UNIT_TEST
    digitalWrite(THETA_DIR_PIN, !reverse);

    // Pulse the step pin
    digitalWrite(THETA_STEP_PIN, HIGH);
    delayMicroseconds(STEP_PULSE_US);
    digitalWrite(THETA_STEP_PIN, LOW);
#else
    printf("theta step %s\n", reverse ? "reverse" : "forward");
#endif
}

void MotorControl::rho_step(bool reverse)
{
    if (!enabled)
    {
        return;
    }

    while (!after_target_timestamp(micros(), rho_next_step_after))
    {
        delayMicroseconds(50);
    }
    rho_next_step_after = micros() + MIN_STEP_DELAY_US;

    if (AXIS_R_INVERT_DIR)
        reverse = !reverse;

#ifndef UNIT_TEST
    digitalWrite(RHO_DIR_PIN, !reverse);

    digitalWrite(RHO_STEP_PIN, HIGH);
    delayMicroseconds(STEP_PULSE_US);
    digitalWrite(RHO_STEP_PIN, LOW);
#else
    printf("Rho step %s\n", reverse ? "reverse" : "forward");
#endif
}

bool MotorControl::get_reference_sensor_value() const
{
#ifndef UNIT_TEST
    // Sensor is active LOW
    return digitalRead(REFERENCE_SENSOR_PIN) == LOW;
#else
    // Under unit tests there is no hardware; return a safe default (not triggered)
    return false;
#endif
}

int8_t MotorControl::update_theta_counter(Direction dir)
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
