#ifndef MOVE_H
#define MOVE_H

enum Direction {
    FORWARD = false,
    BACKWARD = true
};

class Move {
public:
    int thetaSteps;
    int rhoSteps;
    Direction thetaDir;
    Direction rhoDir;
    unsigned long moveTime; // milliseconds

    Move()
        : thetaSteps(0), rhoSteps(0), thetaDir(FORWARD), rhoDir(FORWARD), moveTime(0) {}

    Move(int theta, int rho, Direction tDir, Direction rDir, unsigned long time)
        : thetaSteps(theta), rhoSteps(rho), thetaDir(tDir), rhoDir(rDir), moveTime(time) {}

    int getThetaSteps() const { return thetaSteps; }
    int getRhoSteps() const { return rhoSteps; }
    Direction getThetaDir() const { return thetaDir; }
    Direction getRhoDir() const { return rhoDir; }
    unsigned long getMoveTime() const { return moveTime; }

    void setThetaSteps(int v) { thetaSteps = v; }
    void setRhoSteps(int v) { rhoSteps = v; }
    void setThetaDir(Direction v) { thetaDir = v; }
    void setRhoDir(Direction v) { rhoDir = v; }
    void setMoveTime(unsigned long v) { moveTime = v; }
};

#endif // MOVE_H
