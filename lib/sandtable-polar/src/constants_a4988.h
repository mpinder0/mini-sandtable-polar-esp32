#ifndef CONSTANTS_A4988_H
#define CONSTANTS_A4988_H

// Hardware constants for the A4988 build (bipolar NEMA steppers, 1/16 microstepping)
// Included via constants.h, select with -DMOTOR_DRIVER_A4988

#include "position_helper.h"

// Axis limits
// Theta not limited. in Radians
// Rho, in mm
static constexpr float AXIS_MIN_R = 0.0f;
static constexpr float AXIS_MAX_R = 66.5f;

static constexpr float microsteps_per_step = 16.0f; // 1/16 stepper driver
// Note: theta and rho axis are coupled, so for each theta step rho must also step (theta:rho - 8:1)
static constexpr int THETA_AXIS_GEAR_RATIO = 8; // 8:1 gear ratio

// step size - units
static constexpr float AXIS_STEP_T = deg_to_rad((1.8f / microsteps_per_step) / THETA_AXIS_GEAR_RATIO); // radians
// AXIS_STEP_R = (PI * 10mm) * (1.8deg / 360deg)
static constexpr float AXIS_STEP_R = 0.15707963f / microsteps_per_step; // mm

// speed - units per second
static constexpr float AXIS_SPEED_T = deg_to_rad(80.0f);
static constexpr float AXIS_SPEED_R = 80.0f;
// reference sensor increment - units
static constexpr float AXIS_SPEED_T_REF = deg_to_rad(20.0f);

// Acceleration ramp (see StepMotionPlanner::get_move_steps), in motor steps.
// Start rate: the speed an axis can start at from rest, and the largest instant speed change
// between moves. Tune on the hardware: raise until starts stall, then back off ~30%.
static constexpr float AXIS_START_RATE_T = 800.0f; // steps/s
static constexpr float AXIS_START_RATE_R = 800.0f; // steps/s
// Acceleration above the start rate. Tune by raising until dense patterns lose steps.
static constexpr float AXIS_ACCEL_T = 20000.0f; // steps/s^2
static constexpr float AXIS_ACCEL_R = 20000.0f; // steps/s^2
// Longer moves are split into pieces of at most this many steps, so the ramp builds over
// the pieces rather than one long move starting at full speed
static constexpr int MAX_SEGMENT_STEPS = 50;

// Timing constants
// Minimum delay between steps in microseconds (1 ms = 1000 us). Effectively a speed limit.
static constexpr unsigned long MIN_STEP_DELAY_US = 50;
// Step pulse width in microseconds. A4988 minimum is 1 us high, 1 us low. Kept short as the
// pulse blocks the step loop.
static constexpr unsigned int STEP_PULSE_US = 3;
// Delay after changing DIR before the STEP rising edge, in microseconds. A4988 minimum is 200 ns.
static constexpr unsigned int DIR_SETUP_US = 1;

// Direction inversion flags (set to true if wiring inverts intended direction)
static constexpr bool AXIS_T_INVERT_DIR = true;
static constexpr bool AXIS_R_INVERT_DIR = false;

#endif // CONSTANTS_A4988_H