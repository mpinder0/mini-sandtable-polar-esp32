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

struct Coordinates
{
    float theta;
    float rho;
};

#endif