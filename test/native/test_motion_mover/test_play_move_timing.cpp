#include <unity.h>
#include <cmath>
#include "motor_control_A4988.h"
#include "step_motion_planner.h"
#include "step_motion_mover.h"

// micros() wraps at 2^32 us (~71.6 minutes) on the ESP32, and the mock does the same.
// These tests start moves around the wrap to check play_move() neither stalls nor skips.

static constexpr unsigned long WRAP_US = 0x100000000UL;
// Allowed overrun past the move time: a few loop passes (step pulse + loop delay)
static constexpr unsigned long OVERRUN_US = 10 * (MIN_STEP_DELAY_US + STEP_PULSE_US);

static Move make_move(int theta_steps, int rho_steps, unsigned long move_time_us)
{
    Move mv;
    mv.targetPosition = {0.0f, 0.0f};
    mv.thetaSteps = theta_steps;
    mv.rhoSteps = rho_steps;
    mv.thetaDir = FORWARD;
    mv.rhoDir = FORWARD;
    mv.moveTime = move_time_us;
    return mv;
}

// Plays the move starting at start_us, checks the duration and that every step was taken
static void play_and_check(unsigned long start_us, Move mv)
{
    MotorControlA4988 motor_control;
    StepMotionPlanner planner;
    StepMotionMover mover(motor_control, planner);

    set_time_us(start_us);
    const Coordinates before = mover.get_current_position();
    mover.play_move(mv);
    const unsigned long elapsed = mock_time - start_us; // mock_time itself does not wrap
    const Coordinates after = mover.get_current_position();

    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(mv.moveTime, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(mv.moveTime + OVERRUN_US + mv.thetaSteps * MIN_STEP_DELAY_US + mv.rhoSteps * MIN_STEP_DELAY_US, elapsed);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_STEP_T / 2, mv.thetaSteps * AXIS_STEP_T, after.theta - before.theta);
    TEST_ASSERT_FLOAT_WITHIN(AXIS_STEP_R / 2, mv.rhoSteps * AXIS_STEP_R, after.rho - before.rho);
}

void test_move_away_from_wrap()
{
    play_and_check(1000000UL, make_move(20, 10, 100000UL));
}

void test_move_ending_just_before_wrap()
{
    // Previously the last micros() read wrapped to a small value, still "< end_ts", stalling ~71.6 minutes
    const unsigned long move_time = 100000UL;
    for (unsigned long gap = 50; gap <= 2000; gap += 50)
    {
        play_and_check(WRAP_US - move_time - gap, make_move(20, 10, move_time));
    }
}

void test_move_straddling_wrap()
{
    // Previously end_ts overflowed below start_ts and the move was skipped
    const unsigned long move_time = 100000UL;
    play_and_check(WRAP_US - move_time / 2, make_move(20, 10, move_time));
}

void test_steps_finished_after_move_time()
{
    // Steps that can't fit in the move time (min step delay) are still taken, not dropped
    play_and_check(1000000UL, make_move(10, 0, 0));
}

void setUp() {}
void tearDown() {}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_move_away_from_wrap);
    RUN_TEST(test_move_ending_just_before_wrap);
    RUN_TEST(test_move_straddling_wrap);
    RUN_TEST(test_steps_finished_after_move_time);
    return UNITY_END();
}
