#include "arduino_mock.h"

unsigned long mock_time = 0; // in microseconds

void set_time_ms(unsigned long ms)
{
    mock_time = ms * 1000;
}

void set_time_us(unsigned long us)
{
    mock_time = us;
}

unsigned long millis()
{
    return mock_time / 1000;
}

unsigned long micros()
{
    // Wrap at 32 bits like the ESP32, where unsigned long is 32 bit
    return mock_time & 0xFFFFFFFFUL;
}

void delay(unsigned long ms)
{
    mock_time += ms * 1000;
}

void delayMicroseconds(unsigned int us)
{
    mock_time += us;
}