#ifndef MOVER_H
#define MOVER_H

#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include "abstract_motor_control.h"
#include "step_motion_planner.h"
#include "time_helper.h"
#include <vector>

class StepMotionMover
{
public:
    StepMotionMover(AbstractMotorControl &motor_control, StepMotionPlanner &motion_planner);
    ~StepMotionMover();

    Coordinates get_current_position();
    void reset_theta_position();
    void set_theta_position(float theta);
    // The mover keeps a reference rather than a copy, so new_pattern must stay alive
    // (and unchanged) until play_pattern() has finished.
    void set_pattern(const std::vector<Coordinates> &new_pattern);
    void play_pattern();
    void play_move(Move move);
    // Rotate theta looking for the reference sensor, extending rho after each full rotation.
    // Once found, centres theta on the sensor from both directions, then re-approaches it on rho.
    // Returns true if the reference was found.
    bool reference();

private:
    // Rho extension between each full reference rotation, in mm
    static constexpr float REFERENCE_RHO_INCREMENT = 2.0f;
    // Polling delay per pass of the play_move() loop, in microseconds. Steps can land up to one
    // pass late, so keep this well below the step intervals. MIN_STEP_DELAY_US is enforced
    // separately by the motor control.
    static constexpr unsigned int MOVE_LOOP_DELAY_US = 5;

    AbstractMotorControl &motor_control;
    StepMotionPlanner &planner;
    const std::vector<Coordinates> *pattern = nullptr;
    // Position held as whole steps from the origin, so it never drifts from what the motors did.
    // Floats are only derived from these (get_current_position()), never accumulated.
    int32_t theta_steps_pos = 0;
    int32_t rho_steps_pos = 0;

    void set_position(Coordinates position);
    // Move from the current position to target, split into pieces of at most MAX_SEGMENT_STEPS
    // so the planner's acceleration ramp builds over long moves
    void move_to(Coordinates target);

    void step_axis_if_due(AxisStepper &ax, uint32_t current_ts);
    // Single blocking step for referencing, then wait step_delay_us
    void reference_step(Axis ax, bool reverse, unsigned int step_delay_us);
    // Step until the reference sensor reads sensor_state.
    // Returns the number of steps taken, or -1 if not reached within max_steps.
    int seek_sensor(Axis ax, bool reverse, bool sensor_state, int max_steps, unsigned int step_delay_us);
};

#endif // MOVER_H