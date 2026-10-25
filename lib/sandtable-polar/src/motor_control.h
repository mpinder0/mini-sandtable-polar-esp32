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
    void step(Axis ax, bool reverse = false) override;
    bool get_reference_sensor_value() const override;
    void set_enable(bool enable) override;
    bool is_enabled() const override;

private:
    unsigned long theta_next_step_after;
    unsigned long rho_next_step_after;
    bool enabled;
    int8_t theta_gear_step_counter;

    // Pin definitions
    const int REFERENCE_SENSOR_PIN = 5; // sensor input (active LOW)
    const int THETA_STEP_PIN = 2;
    const int THETA_DIR_PIN = 4;
    const int RHO_STEP_PIN = 16;
    const int RHO_DIR_PIN = 17;
    const int MOTORS_ENABLE_PIN = 15; // active LOW

    void theta_step(bool reverse = false);
    void rho_step(bool reverse = false);
    int8_t update_theta_counter(Direction dir);
};

#endif // MOTOR_CONTROL_H
