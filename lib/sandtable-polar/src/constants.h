#ifndef CONSTANTS_H
#define CONSTANTS_H

// Selects the hardware constants for the motor driver build (set in platformio.ini build_flags).
// Defaults to A4988 when no driver is specified (e.g. the native unit test env).
#if defined(MOTOR_DRIVER_A4988) && defined(MOTOR_DRIVER_ULN2803)
#error "Define only one of MOTOR_DRIVER_A4988 or MOTOR_DRIVER_ULN2803"
#elif defined(MOTOR_DRIVER_ULN2803)
#include "constants_uln2803.h"
#else
#include "constants_a4988.h"
#endif

#endif // CONSTANTS_H
