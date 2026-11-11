#include "motor_control_A4988.h"

// Only built for the A4988 hardware (the default), see constants.h
#ifndef MOTOR_DRIVER_ULN2803

MotorControlA4988::MotorControlA4988()
    : axes{{THETA_STEP_PIN, THETA_DIR_PIN, AXIS_T_INVERT_DIR, "theta"},
           {RHO_STEP_PIN, RHO_DIR_PIN, AXIS_R_INVERT_DIR, "rho"}}
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

    printf("MotorControl A4988 initialized\n");
}

MotorControlA4988::~MotorControlA4988()
{
    // pass
}

void MotorControlA4988::write_enable(bool enable)
{
#ifndef UNIT_TEST
    // Motor drivers are enabled with LOW (active low enable pin)
    digitalWrite(MOTORS_ENABLE_PIN, enable ? LOW : HIGH);
#else
    (void)enable;
#endif
}

void MotorControlA4988::write_step(Axis ax, bool reverse)
{
    const AxisOutput &out = axes[ax];

    if (out.invert_dir)
        reverse = !reverse;

#ifndef UNIT_TEST
    digitalWrite(out.dir_pin, !reverse);

    // Pulse the step pin
    digitalWrite(out.step_pin, HIGH);
    delayMicroseconds(STEP_PULSE_US);
    digitalWrite(out.step_pin, LOW);
#else
    printf("%s step %s\n", out.name, reverse ? "reverse" : "forward");
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

#endif // MOTOR_DRIVER_ULN2803
