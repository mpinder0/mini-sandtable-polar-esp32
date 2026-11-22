#include <unity.h>
#include <cmath>
#include <vector>
#include "motor_control_A4988.h"
#include "step_motion_planner.h"
#include "step_motion_mover.h"

// Acceleration ramp in StepMotionPlanner::get_move_steps() and move splitting in
// StepMotionMover::move_to(). Native tests use the A4988 constants.

static const float MAX_RATE_T = AXIS_SPEED_T / AXIS_STEP_T; // steps/s at full speed
static const float MAX_RATE_R = AXIS_SPEED_R / AXIS_STEP_R;
// moveTime is truncated to whole microseconds, so rates come out fractionally high
static const float RATE_TOLERANCE = 1.01f;

static float rate_of(int steps, const Move &mv)
{
    return steps / (mv.moveTime / 1000000.0f);
}

// Plans a move of theta_steps and rho_steps from the given step position, advancing it
static Move plan_steps(StepMotionPlanner &planner, int &theta_pos, int &rho_pos, int theta_steps, int rho_steps)
{
    const Coordinates from = {theta_pos * AXIS_STEP_T, rho_pos * AXIS_STEP_R};
    theta_pos += theta_steps;
    rho_pos += rho_steps;
    const Coordinates to = {theta_pos * AXIS_STEP_T, rho_pos * AXIS_STEP_R};
    return planner.get_move_steps(from, to);
}

void test_ramp_settings_are_meaningful()
{
    // The ramp only applies if the start rate is below full speed
    TEST_ASSERT_TRUE(AXIS_START_RATE_T < MAX_RATE_T);
    TEST_ASSERT_TRUE(AXIS_START_RATE_R < MAX_RATE_R);
}

void test_theta_starts_at_start_rate()
{
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    const Move mv = plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);
    TEST_ASSERT_EQUAL(MAX_SEGMENT_STEPS, mv.thetaSteps);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_START_RATE_T * 0.01f, AXIS_START_RATE_T, rate_of(mv.thetaSteps, mv));
}

void test_rho_starts_at_start_rate()
{
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    const Move mv = plan_steps(planner, t, r, 0, -MAX_SEGMENT_STEPS);
    TEST_ASSERT_EQUAL(MAX_SEGMENT_STEPS, mv.rhoSteps);
    TEST_ASSERT_EQUAL(BACKWARD, mv.rhoDir);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_START_RATE_R * 0.01f, AXIS_START_RATE_R, rate_of(mv.rhoSteps, mv));
}

void test_ramp_up_within_limits_and_reaches_full_speed()
{
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    float prev = 0.0f;
    float rate = 0.0f;
    for (int i = 0; i < 200; ++i)
    {
        const Move mv = plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);
        rate = rate_of(mv.thetaSteps, mv);

        // Never slower than the move before (no reason to slow), never above full speed
        TEST_ASSERT_TRUE(rate >= prev / RATE_TOLERANCE);
        TEST_ASSERT_TRUE(rate <= MAX_RATE_T * RATE_TOLERANCE);
        // Each jump at most a start rate, and within the acceleration limit (v^2 = u^2 + 2as)
        TEST_ASSERT_TRUE(rate <= (prev + AXIS_START_RATE_T) * RATE_TOLERANCE);
        const float reachable = std::sqrt(prev * prev + 2.0f * AXIS_ACCEL_T * mv.thetaSteps);
        TEST_ASSERT_TRUE(rate <= std::max(reachable, AXIS_START_RATE_T) * RATE_TOLERANCE);
        prev = rate;
    }
    TEST_ASSERT_FLOAT_WITHIN(MAX_RATE_T * 0.01f, MAX_RATE_T, rate);
}

void test_ramp_builds_over_small_moves()
{
    // Many small moves: the acceleration limit rather than the start rate jump applies
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    float prev = 0.0f;
    bool accel_limited = false;
    for (int i = 0; i < 2000; ++i)
    {
        const Move mv = plan_steps(planner, t, r, 2, 0);
        const float rate = rate_of(mv.thetaSteps, mv);
        const float reachable = std::sqrt(prev * prev + 2.0f * AXIS_ACCEL_T * mv.thetaSteps);
        TEST_ASSERT_TRUE(rate <= std::max(reachable, AXIS_START_RATE_T) * RATE_TOLERANCE);
        if (rate > AXIS_START_RATE_T * 1.1f && rate < MAX_RATE_T * 0.9f && rate < prev + AXIS_START_RATE_T * 0.5f)
            accel_limited = true;
        prev = rate;
    }
    TEST_ASSERT_TRUE(accel_limited);
    TEST_ASSERT_FLOAT_WITHIN(MAX_RATE_T * 0.01f, MAX_RATE_T, prev);
}

void test_reversal_restarts_at_start_rate()
{
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    for (int i = 0; i < 200; ++i)
        plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);

    const Move mv = plan_steps(planner, t, r, -MAX_SEGMENT_STEPS, 0);
    TEST_ASSERT_EQUAL(BACKWARD, mv.thetaDir);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_START_RATE_T * 0.01f, AXIS_START_RATE_T, rate_of(mv.thetaSteps, mv));
}

void test_stopped_axis_restarts_at_start_rate()
{
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    for (int i = 0; i < 200; ++i)
        plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);

    // A rho only move stops theta, so theta has to ramp again
    plan_steps(planner, t, r, 0, MAX_SEGMENT_STEPS);
    const Move mv = plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_START_RATE_T * 0.01f, AXIS_START_RATE_T, rate_of(mv.thetaSteps, mv));
}

void test_reset_forgets_speed()
{
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    for (int i = 0; i < 200; ++i)
        plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);

    planner.reset();
    const Move mv = plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_START_RATE_T * 0.01f, AXIS_START_RATE_T, rate_of(mv.thetaSteps, mv));
}

void test_zero_step_move_keeps_speed()
{
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    for (int i = 0; i < 200; ++i)
        plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);

    const Move none = plan_steps(planner, t, r, 0, 0);
    TEST_ASSERT_EQUAL(0, none.moveTime);
    const Move mv = plan_steps(planner, t, r, MAX_SEGMENT_STEPS, 0);
    TEST_ASSERT_FLOAT_WITHIN(MAX_RATE_T * 0.01f, MAX_RATE_T, rate_of(mv.thetaSteps, mv));
}

void test_stretch_keeps_axes_in_ratio()
{
    // From rest, theta is limited to the start rate and rho slows with it
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    const Move mv = plan_steps(planner, t, r, 40, 20);
    TEST_ASSERT_EQUAL(40, mv.thetaSteps);
    TEST_ASSERT_EQUAL(20, mv.rhoSteps);
    const float theta_rate = rate_of(mv.thetaSteps, mv);
    TEST_ASSERT_TRUE(theta_rate <= AXIS_START_RATE_T * RATE_TOLERANCE);
    TEST_ASSERT_FLOAT_WITHIN(theta_rate * 0.01f, theta_rate / 2.0f, rate_of(mv.rhoSteps, mv));
}

void test_full_speed_unchanged_once_ramped()
{
    // Once at full speed, moves take the time the speed limits alone give
    StepMotionPlanner planner;
    int t = 0, r = 1000;
    for (int i = 0; i < 200; ++i)
        plan_steps(planner, t, r, MAX_SEGMENT_STEPS, MAX_SEGMENT_STEPS / 2);

    const Move mv = plan_steps(planner, t, r, MAX_SEGMENT_STEPS, MAX_SEGMENT_STEPS / 2);
    const float unramped_s = std::max(MAX_SEGMENT_STEPS / MAX_RATE_T, (MAX_SEGMENT_STEPS / 2) / MAX_RATE_R);
    TEST_ASSERT_FLOAT_WITHIN(unramped_s * 0.01f, unramped_s, mv.moveTime / 1000000.0f);
}

void test_long_move_is_split_ramped_and_lands_on_target()
{
    MotorControlA4988 motor_control;
    StepMotionPlanner planner;
    StepMotionMover mover(motor_control, planner);
    set_time_us(1000000UL);

    // A single long theta move, far more than one segment
    const int steps = 40 * MAX_SEGMENT_STEPS;
    std::vector<Coordinates> pattern = {{steps * AXIS_STEP_T, 10.0f}};
    mover.set_theta_position(0.0f);
    mover.set_pattern(pattern);

    const unsigned long start = mock_time;
    mover.play_pattern();
    const float elapsed_s = (mock_time - start) / 1000000.0f;

    const Coordinates end = mover.get_current_position();
    TEST_ASSERT_FLOAT_WITHIN(AXIS_STEP_T / 2, pattern[0].theta, end.theta);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_STEP_R / 2, pattern[0].rho, end.rho);

    // Ramped: slower than full speed throughout, by at least the time an ideal ramp at the
    // acceleration limit loses (ramp time minus the time those steps take at full speed),
    // but faster than staying at the start rate
    const float ramp_s = (MAX_RATE_T - AXIS_START_RATE_T) / AXIS_ACCEL_T;
    const float ramp_steps = (MAX_RATE_T * MAX_RATE_T - AXIS_START_RATE_T * AXIS_START_RATE_T) / (2.0f * AXIS_ACCEL_T);
    const float ramp_loss_s = ramp_s - ramp_steps / MAX_RATE_T;
    TEST_ASSERT_TRUE(elapsed_s > steps / MAX_RATE_T + ramp_loss_s);
    TEST_ASSERT_TRUE(elapsed_s < steps / AXIS_START_RATE_T);
}

void setUp() {}
void tearDown() {}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_ramp_settings_are_meaningful);
    RUN_TEST(test_theta_starts_at_start_rate);
    RUN_TEST(test_rho_starts_at_start_rate);
    RUN_TEST(test_ramp_up_within_limits_and_reaches_full_speed);
    RUN_TEST(test_ramp_builds_over_small_moves);
    RUN_TEST(test_reversal_restarts_at_start_rate);
    RUN_TEST(test_stopped_axis_restarts_at_start_rate);
    RUN_TEST(test_reset_forgets_speed);
    RUN_TEST(test_zero_step_move_keeps_speed);
    RUN_TEST(test_stretch_keeps_axes_in_ratio);
    RUN_TEST(test_full_speed_unchanged_once_ramped);
    RUN_TEST(test_long_move_is_split_ramped_and_lands_on_target);
    return UNITY_END();
}
