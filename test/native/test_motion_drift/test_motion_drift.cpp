#include <unity.h>
#include <cmath>
#include <vector>
#include "motor_control_A4988.h"
#include "step_motion_planner.h"
#include "step_motion_mover.h"

// Patterns made of many moves smaller than a step used to round to zero steps per move,
// so the table drifted from (or never reached) the pattern. Steps are now taken from
// positions snapped to the step grid, and planned from the actual step position.

void test_planner_steps_telescope()
{
    // 0.4 of a step per move: rounding each delta gives 0 every time
    StepMotionPlanner planner;
    int total_theta = 0;
    int total_rho = 0;
    for (int i = 0; i < 100; ++i)
    {
        const Coordinates from = {i * 0.4f * AXIS_STEP_T, i * 0.4f * AXIS_STEP_R};
        const Coordinates to = {(i + 1) * 0.4f * AXIS_STEP_T, (i + 1) * 0.4f * AXIS_STEP_R};
        const Move mv = planner.get_move_steps(from, to);
        total_theta += (mv.thetaDir == FORWARD) ? mv.thetaSteps : -mv.thetaSteps;
        total_rho += (mv.rhoDir == FORWARD) ? mv.rhoSteps : -mv.rhoSteps;
    }
    TEST_ASSERT_EQUAL(40, total_theta);
    TEST_ASSERT_EQUAL(40, total_rho);
}

void test_pattern_of_sub_step_moves_reaches_end()
{
    MotorControlA4988 motor_control;
    StepMotionPlanner planner;
    StepMotionMover mover(motor_control, planner);
    set_time_us(1000000UL);

    std::vector<Coordinates> pattern;
    for (int i = 0; i <= 250; ++i)
        pattern.push_back({i * 0.3f * AXIS_STEP_T, 10.0f + i * 0.3f * AXIS_STEP_R});

    mover.set_pattern(pattern);
    mover.play_pattern();

    const Coordinates end = mover.get_current_position();
    TEST_ASSERT_FLOAT_WITHIN(AXIS_STEP_T / 2, pattern.back().theta, end.theta);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_STEP_R / 2, pattern.back().rho, end.rho);
}

void test_reset_theta_removes_whole_rotations()
{
    MotorControlA4988 motor_control;
    StepMotionPlanner planner;
    StepMotionMover mover(motor_control, planner);

    const float two_pi = 2.0f * static_cast<float>(PI);
    mover.set_theta_position(5 * two_pi + 1.0f);
    mover.reset_theta_position();
    TEST_ASSERT_FLOAT_WITHIN(AXIS_STEP_T, 1.0f, mover.get_current_position().theta);

    mover.set_theta_position(-3 * two_pi - 1.0f);
    mover.reset_theta_position();
    TEST_ASSERT_FLOAT_WITHIN(AXIS_STEP_T, -1.0f, mover.get_current_position().theta);
}

void setUp() {}
void tearDown() {}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_planner_steps_telescope);
    RUN_TEST(test_pattern_of_sub_step_moves_reaches_end);
    RUN_TEST(test_reset_theta_removes_whole_rotations);
    return UNITY_END();
}
