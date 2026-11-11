#ifndef MOTOR_CONTROL_A4988_H
#define MOTOR_CONTROL_A4988_H

#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include "structs.h"
#include "abstract_motor_control.h"
#include "constants.h"

class MotorControlA4988 : public AbstractMotorControl
{
public:
    MotorControlA4988();
    ~MotorControlA4988();

    bool get_reference_sensor_value() const override;

protected:
    void write_step(Axis ax, bool reverse) override;
    void write_enable(bool enable) override;

private:
    // Per-axis output pins
    struct AxisOutput
    {
        int step_pin;
        int dir_pin;
        bool invert_dir;
        const char *name;
    };

    // Pin definitions
    static constexpr int REFERENCE_SENSOR_PIN = 5; // sensor input (active LOW)
    static constexpr int THETA_STEP_PIN = 2;
    static constexpr int THETA_DIR_PIN = 4;
    static constexpr int RHO_STEP_PIN = 16;
    static constexpr int RHO_DIR_PIN = 17;
    static constexpr int MOTORS_ENABLE_PIN = 15; // active LOW

    AxisOutput axes[2]; // indexed by Axis
};

#endif // MOTOR_CONTROL_A4988_H
