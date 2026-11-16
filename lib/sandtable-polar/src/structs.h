#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

enum Direction
{
    FORWARD = false,
    BACKWARD = true
};

// Axis enum (theta/rho)
enum Axis
{
    THETA,
    RHO
};

// Per-axis stepping state while executing a move
struct AxisStepper
{
    Axis axis;
    Direction dir;
    int total_steps;
    uint32_t interval; // microseconds between steps
    uint32_t next_ts;  // micros() timestamp of next step
    int steps_done;
};

struct Coordinates
{
    float theta;
    float rho;
};

struct Move
{
    Coordinates targetPosition;
    int thetaSteps;
    int rhoSteps;
    Direction thetaDir;
    Direction rhoDir;
    unsigned long moveTime; // microseconds
};

#endif