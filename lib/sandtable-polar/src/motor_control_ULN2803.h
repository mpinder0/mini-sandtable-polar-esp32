#ifndef MOTOR_CONTROL_ULN2803_H
#define MOTOR_CONTROL_ULN2803_H

#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include "structs.h"
#include "time_helper.h"
#include "abstract_motor_control.h"
#include "constants.h"

class MotorControlULN2803 : public AbstractMotorControl
{
public:
    MotorControlULN2803();
    ~MotorControlULN2803();

    void motors_release() override;
    bool step(Axis ax, bool reverse = false) override;
    bool get_reference_sensor_value() const override;
    void set_enable(bool enable) override;
    bool is_enabled() const override;

private:
    // Per-axis output pins and step timing
    struct AxisOutput
    {
        const uint8_t *coil_pins; // 4 ULN2803 inputs driving the motor coils
        bool invert_dir;
        const char *name;
        unsigned long next_step_after; // micros() timestamp
        int8_t sequence_index;         // current row of HALF_STEP_SEQUENCE
    };

    // Pin definitions
    static constexpr uint8_t REFERENCE_SENSOR_PIN = 23;             // sensor input (active LOW)
    static constexpr uint8_t THETA_STEP_PINS[4] = {27, 26, 25, 33}; // Array of step pins for theta axis
    static constexpr uint8_t RHO_STEP_PINS[4] = {18, 19, 21, 22};   // Array of step pins for rho axis
    static constexpr int8_t SEQUENCE_LENGTH = 8;
    static constexpr uint8_t HALF_STEP_SEQUENCE[SEQUENCE_LENGTH][4] = {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1},
    };

    AxisOutput theta_axis;
    AxisOutput rho_axis;
    bool enabled;
    int8_t theta_gear_step_counter;

    bool theta_step(bool reverse = false);
    bool rho_step(bool reverse = false);
    bool axis_ready(const AxisOutput &ax) const;
    void pulse_axis(AxisOutput &ax, bool reverse);
    void write_coils(const AxisOutput &ax, bool energise) const;
    int8_t update_theta_counter(Direction dir);
};

#endif // MOTOR_CONTROL_ULN2803_H
