#ifndef POSITION_HELPER_H
#define POSITION_HELPER_H

// defined in Arduino.h
// const float PI = 3.141592653589793f;
#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif
#include <cmath>
#include <stdint.h>

static bool position_at_target(float one_position, float target, float tolerance = 0.1f)
{
    return (one_position >= target - tolerance && one_position <= target + tolerance);
}

static constexpr float deg_to_rad(float deg)
{
    // Convert degrees to radians.
    return deg * (static_cast<float>(PI) / 180.0f);
}

static constexpr float rad_to_deg(float rad)
{
    // Convert radians to degrees.
    return rad * (180.0f / static_cast<float>(PI));
}

// Position snapped to the nearest whole step. Double precision so large theta values
// (many rotations) still snap correctly, it's only called once per move.
static inline int32_t position_to_steps(float position, float step_size)
{
    return static_cast<int32_t>(std::lround(static_cast<double>(position) / static_cast<double>(step_size)));
}

#endif // POSITION_HELPER_H
