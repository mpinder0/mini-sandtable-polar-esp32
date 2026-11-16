#ifndef TIME_HELPER_H
#define TIME_HELPER_H

#include <stdint.h>

// micros() timestamps are 32 bit on the ESP32 and wrap every 2^32 us (~71.6 minutes).
// Always compare timestamps through these helpers (or an unsigned difference),
// never with a plain < or >, which breaks across the wrap.

inline bool after_target_timestamp(uint32_t current_time, uint32_t target_time)
{
    // Handle wrap-around by using time delta, unsigned arithmetic
    const uint32_t delta = target_time - current_time;
    // Cast as signed to test for negative (overflowed)
    return static_cast<int32_t>(delta) <= 0;
}

// Microseconds since start_time, correct across a micros() wrap
inline uint32_t elapsed_since(uint32_t current_time, uint32_t start_time)
{
    return current_time - start_time;
}

#endif // TIME_HELPER_H
