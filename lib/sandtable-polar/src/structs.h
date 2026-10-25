#ifndef STRUCTS_H
#define STRUCTS_H

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

struct Move
{
    int thetaSteps;
    int rhoSteps;
    Direction thetaDir;
    Direction rhoDir;
    unsigned long moveTime; // milliseconds
};

// Per-axis stepping state while executing a move
struct AxisStepper
{
    Axis axis;
    Direction dir;
    int total_steps;
    unsigned long interval; // milliseconds between steps
    unsigned long next_ts;  // timestamp of next step
    int steps_done;
};

struct Coordinates
{
    float theta;
    float rho;
};

#endif