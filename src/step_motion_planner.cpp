#include "step_motion_planner.h"
#include <cmath>

StepMotionPlanner::StepMotionPlanner() {
  // Constructor implementation (if needed)
}

StepMotionPlanner::~StepMotionPlanner() {
  // Destructor implementation (if needed)
}

Move StepMotionPlanner::get_move_steps(Coordinates start_pos, Coordinates end_pos) {
  Move move;

  // Calculate the difference in position
  float delta_theta = end_pos.theta - start_pos.theta;
  float delta_rho = end_pos.rho - start_pos.rho;

  // Calculate the number of steps for each axis
  int steps_theta = static_cast<int>(std::round(delta_theta / AXIS_STEP_T));
  int steps_rho = static_cast<int>(std::round(delta_rho / AXIS_STEP_R));

  // Determine direction for each axis
  move.thetaDir = (steps_theta < 0) ? BACKWARD : FORWARD;
  move.rhoDir = (steps_rho < 0) ? BACKWARD : FORWARD;

  // Store the absolute number of steps
  move.thetaSteps = std::abs(steps_theta);
  move.rhoSteps = std::abs(steps_rho);

  // Calculate move time for each axis
  float move_time_theta = static_cast<float>(delta_theta) / AXIS_SPEED_T;
  float move_time_rho = static_cast<float>(delta_rho) / AXIS_SPEED_R;
  // Use the maximum time required for either axis (for slowest axis)
  move.moveTime = (float)std::max(move_time_theta, move_time_rho) * 1000.0f; // Convert to milliseconds

  return move;
}