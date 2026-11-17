#ifndef ABSTRACT_MOTOR_CONTROL_H
#define ABSTRACT_MOTOR_CONTROL_H

#include <stdint.h>
#include "structs.h"

// Shared stepping logic (enable state, step timing, theta/rho gear coupling).
// Drivers implement the hardware hooks write_step() and write_enable().
class AbstractMotorControl
{
public:
    virtual ~AbstractMotorControl() = default;

    void motors_release();
    // Steps the axis if its minimum step delay has elapsed.
    // Returns false without stepping if called too soon; the caller should retry.
    bool step(Axis ax, bool reverse = false);
    void set_enable(bool enable);
    bool is_enabled() const;
    virtual bool get_reference_sensor_value() const = 0;

protected:
    // Hardware hooks. Not safe to call from the base constructor, so drivers
    // set their initial output state in their own constructor.
    virtual void write_step(Axis ax, bool reverse) = 0; // output one step on the axis
    virtual void write_enable(bool enable) = 0;         // energise / de-energise the motors

private:
    bool enabled = false;
    int8_t theta_gear_step_counter = 0;
    // micros() of each axis's last step and whether it has stepped yet, indexed by Axis.
    // Readiness is the elapsed time since the last step, which stays correct across a micros()
    // wrap however long the axis has been idle. (Comparing against a stored "next step" time
    // breaks once it is more than 2^31 us stale: it then looks like the future.)
    uint32_t last_step_ts[2] = {0, 0};
    bool has_stepped[2] = {false, false};

    bool theta_step(bool reverse);
    bool rho_step(bool reverse);
    bool axis_ready(Axis ax) const;
    void pulse_axis(Axis ax, bool reverse);
    int8_t update_theta_counter(Direction dir);
};

#endif // ABSTRACT_MOTOR_CONTROL_H
