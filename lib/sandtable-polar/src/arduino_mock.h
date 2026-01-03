#ifndef ARDUINO_H
#define ARDUINO_H

#include <chrono>
#include <thread>

#define PI 3.141592653589793f

static unsigned long millis()
{
    using namespace std::chrono;
    static auto start = steady_clock::now();
    return (unsigned long)duration_cast<milliseconds>(steady_clock::now() - start).count();
}

static void delay(unsigned long ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

static void delayMicroseconds(unsigned int us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

#endif // ARDUINO_H