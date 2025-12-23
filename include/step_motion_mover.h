#ifndef MOVER_H
#define MOVER_H

#include <Arduino.h>
#include "motor_control.h"

class StepMotionMover {
public:
  StepMotionMover(MotorControl& motor_control);
  ~StepMotionMover();
 
  void play();
  void play_move();

private:
  unsigned long theta_timestamp_ms;

};

#endif // MOVER_H


/*
    def play_move(self, move):
        '''
        params
        move - dict:
            start_position - tuple (theta, rho) current position
            end_position - tuple (theta, rho) next position
            axis_steps_list - list of tuples (theta, rho) contraining step True/False for each time step
            directions - tuple F/R for each axis
        '''
        axis_steps_list = move['axis_steps_list']
        directions = move['directions']

        # iterate through the axis steps list
        for step in axis_steps_list:
            # execute the axis steps
            self._play_both_axis_step(directions, step)
            # wait for time step
            time.sleep(TIME_STEP_S)
        
        self.current_position = move["end_position"]
    
    def play(self, pattern):
        # loop through position changes
        for position in pattern:
            move = self.get_steps_for_move(position)
            # execute motor steps to complete the move
            self.play_move(move)
*/