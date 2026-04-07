#include "arduino_mock.h"

unsigned long mock_time = 0; // in microseconds

void set_time_ms(unsigned long ms)
{
    mock_time = ms * 1000;
}

unsigned long millis()
{
    return mock_time / 1000;
}

unsigned long micros()
{
    return mock_time;
}

void delay(unsigned long ms)
{
    mock_time += ms * 1000;
}

void delayMicroseconds(unsigned int us)
{
    mock_time += us;
}