#ifndef ARDUINO_H
#define ARDUINO_H

#include <chrono>
#include <thread>

#define PI 3.141592653589793f

extern unsigned long mock_time; // in microseconds

void set_time_ms(unsigned long ms);
unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

#endif // ARDUINO_H