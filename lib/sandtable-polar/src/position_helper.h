#ifndef POSITION_HELPER_H
#define POSITION_HELPER_H

// defined in Arduino.h
// const float PI = 3.141592653589793f;
#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <arduino_mock.h>
#endif

static bool position_at_target(float one_position, float target, float tolerance = 0.1f)
{
    return (one_position >= target - tolerance && one_position <= target + tolerance);
}

static float deg_to_rad(float deg)
{
    // Convert degrees to radians.
    return deg * (static_cast<float>(PI) / 180.0f);
}

static float rad_to_deg(float rad)
{
    // Convert radians to degrees.
    return rad * (180.0f / static_cast<float>(PI));
}

#endif // POSITION_HELPER_H
