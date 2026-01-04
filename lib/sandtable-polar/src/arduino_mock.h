#ifndef ARDUINO_H
#define ARDUINO_H

#include <chrono>
#include <thread>

#define PI 3.141592653589793f

inline unsigned long mock_time = 0; // in microseconds

inline void set_time_ms(unsigned long ms)
{
    mock_time = ms * 1000;
}

inline unsigned long millis()
{
    return mock_time / 1000;
}

inline unsigned long micros()
{
    return mock_time;
}

inline void delay(unsigned long ms)
{
    mock_time += ms * 1000;
}

inline void delayMicroseconds(unsigned int us)
{
    mock_time += us;
}

#endif // ARDUINO_H