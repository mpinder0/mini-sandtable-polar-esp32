#include "motor_control_ULN2803.h"

// Only built for the ULN2803 hardware, see constants.h
#ifdef MOTOR_DRIVER_ULN2803

// Out-of-line definitions for ODR-used constexpr arrays (required before C++17)
constexpr uint8_t MotorControlULN2803::THETA_STEP_PINS[4];
constexpr uint8_t MotorControlULN2803::RHO_STEP_PINS[4];
constexpr uint8_t MotorControlULN2803::HALF_STEP_SEQUENCE[MotorControlULN2803::SEQUENCE_LENGTH][4];

MotorControlULN2803::MotorControlULN2803()
    : axes{{THETA_STEP_PINS, AXIS_T_INVERT_DIR, "theta", 0},
           {RHO_STEP_PINS, AXIS_R_INVERT_DIR, "rho", 0}}
{
    // Configure pins (skipped under UNIT_TEST)
#ifndef UNIT_TEST
    pinMode(REFERENCE_SENSOR_PIN, INPUT);
    for (int i = 0; i < 4; i++)
    {
        pinMode(THETA_STEP_PINS[i], OUTPUT);
        pinMode(RHO_STEP_PINS[i], OUTPUT);
    }
#endif

    // Initialize outputs to a safe state (all coils off)
    set_enable(false);

    printf("MotorControl ULN2803 unipolar initialized\n");
}

MotorControlULN2803::~MotorControlULN2803()
{
    motors_release();
}

void MotorControlULN2803::write_enable(bool enable)
{
    // No enable pin on the ULN2803: enabling re-energises the coils at the
    // current sequence position (holding torque), disabling turns all coils off.
    write_coils(axes[THETA], enable);
    write_coils(axes[RHO], enable);
}

void MotorControlULN2803::write_step(Axis ax, bool reverse)
{
    AxisOutput &out = axes[ax];

    if (out.invert_dir)
        reverse = !reverse;

    // Advance one half step through the coil sequence, wrapping at either end
    out.sequence_index += reverse ? -1 : 1;
    if (out.sequence_index >= SEQUENCE_LENGTH)
        out.sequence_index = 0;
    else if (out.sequence_index < 0)
        out.sequence_index = SEQUENCE_LENGTH - 1;

#ifndef UNIT_TEST
    write_coils(out, true);
#else
    printf("%s step %s\n", out.name, reverse ? "reverse" : "forward");
#endif
}

void MotorControlULN2803::write_coils(const AxisOutput &out, bool energise) const
{
#ifndef UNIT_TEST
    const uint8_t *phase = HALF_STEP_SEQUENCE[out.sequence_index];
    for (int i = 0; i < 4; i++)
    {
        digitalWrite(out.coil_pins[i], (energise && phase[i]) ? HIGH : LOW);
    }
#else
    (void)out;
    (void)energise;
#endif
}

bool MotorControlULN2803::get_reference_sensor_value() const
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
