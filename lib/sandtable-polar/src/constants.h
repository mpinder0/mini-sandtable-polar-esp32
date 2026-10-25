#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "position_helper.h"

// Axis limits
// Theta not limited. in Radians
// Rho, in mm
static constexpr float AXIS_MIN_R = 0.0f;
static constexpr float AXIS_MAX_R = 66.5f;

static constexpr float microsteps_per_step = 16.0f; // 1/4 stepper driver
// Note: theta and rho axis are coupled, so for each theta step rho must also step (theta:rho - 8:1)
static constexpr int THETA_AXIS_GEAR_RATIO = 8; // 8:1 gear ratio

// step size - units
static constexpr float AXIS_STEP_T = deg_to_rad((1.8f / microsteps_per_step) / THETA_AXIS_GEAR_RATIO); // radians
// AXIS_STEP_R = (PI * 10mm) * (1.8deg / 360deg)
static constexpr float AXIS_STEP_R = 0.15707963f / microsteps_per_step; // mm

// speed - units per second
static constexpr float AXIS_SPEED_T = deg_to_rad(5.0f);
static constexpr float AXIS_SPEED_R = 5.0f;
// reference sensor increment - units
static constexpr float AXIS_SPEED_T_REF = deg_to_rad(10.0f);

// Timing constants
// Minimum delay between steps in microseconds (1 ms = 1000 us). Effectively a speed limit.
static constexpr unsigned long MIN_STEP_DELAY_US = 750;
// Step pulse width in microseconds (1 ms = 1000 us)
static constexpr unsigned int STEP_PULSE_US = 150;

// Direction inversion flags (set to true if wiring inverts intended direction)
static constexpr bool AXIS_T_INVERT_DIR = false;
static constexpr bool AXIS_R_INVERT_DIR = false;

#endif // CONSTANTS_H