#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include "structs.h"
#include "time_helper.h"
#include "abstract_motor_control.h"
#include "constants.h"

class MotorControl : public AbstractMotorControl
{
public:
    MotorControl();
    ~MotorControl();

    void motors_release() override;
    bool step(Axis ax, bool reverse = false) override;
    bool get_reference_sensor_value() const override;
    void set_enable(bool enable) override;
    bool is_enabled() const override;

private:
    // Per-axis output pins and step timing
    struct AxisOutput
    {
        int step_pin;
        int dir_pin;
        bool invert_dir;
        const char *name;
        unsigned long next_step_after; // micros() timestamp
    };

    // Pin definitions
    static constexpr int REFERENCE_SENSOR_PIN = 5; // sensor input (active LOW)
    static constexpr int THETA_STEP_PIN = 2;
    static constexpr int THETA_DIR_PIN = 4;
    static constexpr int RHO_STEP_PIN = 16;
    static constexpr int RHO_DIR_PIN = 17;
    static constexpr int MOTORS_ENABLE_PIN = 15; // active LOW

    AxisOutput theta_axis;
    AxisOutput rho_axis;
    bool enabled;
    int8_t theta_gear_step_counter;

    bool theta_step(bool reverse = false);
    bool rho_step(bool reverse = false);
    bool axis_ready(const AxisOutput &ax) const;
    void pulse_axis(AxisOutput &ax, bool reverse);
    int8_t update_theta_counter(Direction dir);
};

#endif // MOTOR_CONTROL_H
