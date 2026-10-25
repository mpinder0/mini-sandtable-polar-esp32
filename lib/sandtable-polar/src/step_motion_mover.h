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
    void set_pattern(std::vector<Coordinates> &new_pattern);
    void play_pattern();
    void play_move(Move move);
    // Rotate theta looking for the reference sensor, extending rho after each full rotation.
    // Once found, centres theta on the sensor from both directions, then re-approaches it on rho.
    // Returns true if the reference was found.
    bool reference();

private:
    // Rho extension between each full reference rotation, in mm
    static constexpr float REFERENCE_RHO_INCREMENT = 2.0f;

    AbstractMotorControl &motor_control;
    StepMotionPlanner &planner;
    std::vector<Coordinates> pattern;
    Coordinates current_position = {0, 0};

    void step_axis_if_due(AxisStepper &ax, unsigned long current_ts);
    // Single blocking step for referencing, then wait step_delay_us
    void reference_step(Axis ax, bool reverse, unsigned int step_delay_us);
    // Step until the reference sensor reads sensor_state.
    // Returns the number of steps taken, or -1 if not reached within max_steps.
    int seek_sensor(Axis ax, bool reverse, bool sensor_state, int max_steps, unsigned int step_delay_us);
};

#endif // MOVER_H