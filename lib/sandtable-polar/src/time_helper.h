#ifndef TIME_HELPER_H
#define TIME_HELPER_H

inline bool at_target_timestamp(unsigned long current_time, unsigned long target_time)
{
    // Handle wrap-around by using time delta, unsigned arithmetic
    const auto delta = target_time - current_time;
    // Cast as signed to test for negative (overflowed)
    return static_cast<long>(delta) <= 0;
}

#endif // TIME_HELPER_H
