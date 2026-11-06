#include "motor_control_ULN2803.h"

// Only built for the ULN2803 hardware, see constants.h
#ifdef MOTOR_DRIVER_ULN2803

// Out-of-line definitions for ODR-used constexpr arrays (required before C++17)
constexpr uint8_t MotorControlULN2803::THETA_STEP_PINS[4];
constexpr uint8_t MotorControlULN2803::RHO_STEP_PINS[4];
constexpr uint8_t MotorControlULN2803::HALF_STEP_SEQUENCE[MotorControlULN2803::SEQUENCE_LENGTH][4];

MotorControlULN2803::MotorControlULN2803()
    : theta_axis{THETA_STEP_PINS, AXIS_T_INVERT_DIR, "theta", 0, 0},
      rho_axis{RHO_STEP_PINS, AXIS_R_INVERT_DIR, "rho", 0, 0},
      enabled(false), theta_gear_step_counter(0)
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

void MotorControlULN2803::set_enable(bool enable)
{
    // No enable pin on the ULN2803: enabling re-energises the coils at the
    // current sequence position (holding torque), disabling turns all coils off.
    enabled = enable;
    write_coils(theta_axis, enable);
    write_coils(rho_axis, enable);
}

bool MotorControlULN2803::is_enabled() const
{
    return enabled;
}

void MotorControlULN2803::motors_release()
{
    set_enable(false);
}

bool MotorControlULN2803::step(Axis ax, bool reverse)
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

bool MotorControlULN2803::theta_step(bool reverse)
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

bool MotorControlULN2803::rho_step(bool reverse)
{
    if (!axis_ready(rho_axis))
    {
        return false;
    }

    pulse_axis(rho_axis, reverse);
    return true;
}

bool MotorControlULN2803::axis_ready(const AxisOutput &ax) const
{
    // Motors enabled and minimum step delay elapsed since the last step
    return enabled && after_target_timestamp(micros(), ax.next_step_after);
}

void MotorControlULN2803::pulse_axis(AxisOutput &ax, bool reverse)
{
    ax.next_step_after = micros() + MIN_STEP_DELAY_US;

    if (ax.invert_dir)
        reverse = !reverse;

    // Advance one half step through the coil sequence, wrapping at either end
    ax.sequence_index += reverse ? -1 : 1;
    if (ax.sequence_index >= SEQUENCE_LENGTH)
        ax.sequence_index = 0;
    else if (ax.sequence_index < 0)
        ax.sequence_index = SEQUENCE_LENGTH - 1;

#ifndef UNIT_TEST
    write_coils(ax, true);
#else
    printf("%s step %s\n", ax.name, reverse ? "reverse" : "forward");
#endif
}

void MotorControlULN2803::write_coils(const AxisOutput &ax, bool energise) const
{
#ifndef UNIT_TEST
    const uint8_t *phase = HALF_STEP_SEQUENCE[ax.sequence_index];
    for (int i = 0; i < 4; i++)
    {
        digitalWrite(ax.coil_pins[i], (energise && phase[i]) ? HIGH : LOW);
    }
#else
    (void)ax;
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

int8_t MotorControlULN2803::update_theta_counter(Direction dir)
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
