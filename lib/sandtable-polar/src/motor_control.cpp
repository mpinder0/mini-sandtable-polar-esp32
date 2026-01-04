#include "motor_control.h"

MotorControl::MotorControl()
    : theta_next_step_after(0), rho_next_step_after(0), enabled(false)
{
    // Configure pins (skipped under UNIT_TEST)
#ifndef UNIT_TEST
    pinMode(REFERENCE_SENSOR_PIN, INPUT_PULLUP);
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

MotorControl::~MotorControl() {}

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
    while (!at_target_timestamp(millis(), theta_next_step_after))
    {
        delayMicroseconds(200);
    }
    theta_next_step_after = millis() + MIN_STEP_DELAY_MS;

    if (AXIS_T_INVERT_DIR)
        reverse = !reverse;

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

    while (!at_target_timestamp(millis(), rho_next_step_after))
    {
        delayMicroseconds(200);
    }
    rho_next_step_after = millis() + MIN_STEP_DELAY_MS;

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
