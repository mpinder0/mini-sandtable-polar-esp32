#ifndef ABSTRACT_MOTOR_CONTROL_H
#define ABSTRACT_MOTOR_CONTROL_H

#include <structs.h>

class AbstractMotorControl
{
public:
    virtual ~AbstractMotorControl() = default;

    virtual void motors_release() = 0;
    virtual void step(Axis ax, bool reverse = false) = 0;
    virtual bool get_reference_sensor_value() const = 0;
    virtual void set_enable(bool enable) = 0;
    virtual bool is_enabled() const = 0;
};

#endif // ABSTRACT_MOTOR_CONTROL_H