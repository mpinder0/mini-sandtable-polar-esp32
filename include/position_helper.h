#ifndef POSITION_HELPER_H
#define POSITION_HELPER_H

// defined in Arduino.h
// const float PI = 3.141592653589793f;

bool position_at_target(float one_position, float target, float tolerance = 0.1f)
{
    return (one_position >= target - tolerance && one_position <= target + tolerance);
}

float deg_to_rad(float deg)
{
    // Convert degrees to radians.
    return deg * (PI / 180.0f);
}

float rad_to_deg(float rad)
{
    // Convert radians to degrees.
    return rad * (180.0f / PI);
}

#endif // POSITION_HELPER_H
