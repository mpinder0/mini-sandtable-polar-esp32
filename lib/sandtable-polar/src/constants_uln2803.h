#ifndef CONSTANTS_ULN2803_H
#define CONSTANTS_ULN2803_H

// Hardware constants for the ULN2803 build (unipolar steppers, half stepping)
// Included via constants.h, select with -DMOTOR_DRIVER_ULN2803

#include "position_helper.h"

// Axis limits
// Theta not limited. in Radians
// Rho, in mm
static constexpr float AXIS_MIN_R = 0.0f;
static constexpr float AXIS_MAX_R = 66.5f;

// TODO: confirm for the fitted motor. 28BYJ-48: 64 steps/rev * 64:1 gearbox, half stepping
static constexpr float steps_per_motor_rev = 4096.0f;
// Note: theta and rho axis are coupled, so for each theta step rho must also step (theta:rho - 8:1)
static constexpr int THETA_AXIS_GEAR_RATIO = 8; // 8:1 gear ratio

// step size - units
static constexpr float AXIS_STEP_T = deg_to_rad((360.0f / steps_per_motor_rev) / THETA_AXIS_GEAR_RATIO); // radians
// AXIS_STEP_R = (PI * 10mm) / steps_per_motor_rev
static constexpr float AXIS_STEP_R = 31.415927f / steps_per_motor_rev; // mm

// speed - units per second
static constexpr float AXIS_SPEED_T = deg_to_rad(5.0f);
static constexpr float AXIS_SPEED_R = 5.0f;
// reference sensor increment - units
static constexpr float AXIS_SPEED_T_REF = deg_to_rad(10.0f);

// Timing constants
// Minimum delay between steps in microseconds (1 ms = 1000 us). Effectively a speed limit.
// TODO: tune on hardware, 28BYJ-48 typically skips below ~1-2 ms per half step
static constexpr unsigned long MIN_STEP_DELAY_US = 2000;

// Direction inversion flags (set to true if wiring inverts intended direction)
static constexpr bool AXIS_T_INVERT_DIR = false;
static constexpr bool AXIS_R_INVERT_DIR = false;

#endif // CONSTANTS_ULN2803_H
