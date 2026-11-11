#include <unity.h>
#include "motor_control_A4988.h"

MotorControlA4988 motorControl;

void test_enable()
{
    TEST_ASSERT_FALSE(motorControl.is_enabled());
    motorControl.set_enable(true);
    TEST_ASSERT_TRUE(motorControl.is_enabled());
}

void test_enabled_on_step()
{
    // reset
    motorControl.set_enable(false);
    // theta Theta
    TEST_ASSERT_FALSE(motorControl.is_enabled());
    motorControl.step(THETA);
    TEST_ASSERT_TRUE(motorControl.is_enabled());

    // reset
    motorControl.set_enable(false);
    // test for Rho
    TEST_ASSERT_FALSE(motorControl.is_enabled());
    motorControl.step(RHO);
    TEST_ASSERT_TRUE(motorControl.is_enabled());
}

void test_disable()
{
    motorControl.set_enable(true);
    TEST_ASSERT_TRUE(motorControl.is_enabled());
    motorControl.motors_release();
    TEST_ASSERT_FALSE(motorControl.is_enabled());
}

void test_step_min_time()
{
    motorControl.set_enable(true);

    set_time_us(1000);

    TEST_ASSERT_TRUE(motorControl.step(THETA));
    // Too soon, refused without blocking
    TEST_ASSERT_FALSE(motorControl.step(THETA));
    TEST_ASSERT_EQUAL(1000, micros());

    // Just before the minimum delay
    delayMicroseconds(MIN_STEP_DELAY_US - 1);
    TEST_ASSERT_FALSE(motorControl.step(THETA));

    // Minimum delay elapsed
    delayMicroseconds(1);
    TEST_ASSERT_TRUE(motorControl.step(THETA));
}

// Steps theta n times, waiting the minimum delay before each step
static void step_theta_n(int n)
{
    for (int i = 0; i < n; i++)
    {
        delayMicroseconds(MIN_STEP_DELAY_US);
        TEST_ASSERT_TRUE(motorControl.step(THETA));
    }
}

void test_theta_coupled_rho_step()
{
    set_time_us(1000);

    // No coupled rho step before a full gear ratio of theta steps (so rho is still free to step)
    step_theta_n(THETA_AXIS_GEAR_RATIO - 1);
    TEST_ASSERT_TRUE(motorControl.step(RHO));

    // Full gear ratio reached, rho steps with theta (so a direct rho step is too soon)
    step_theta_n(1);
    TEST_ASSERT_FALSE(motorControl.step(RHO));
}

void test_theta_waits_for_coupled_rho()
{
    set_time_us(1000);
    step_theta_n(THETA_AXIS_GEAR_RATIO - 1);

    // Rho just stepped, so the theta step needing a coupled rho step is refused
    delayMicroseconds(MIN_STEP_DELAY_US);
    TEST_ASSERT_TRUE(motorControl.step(RHO));
    TEST_ASSERT_FALSE(motorControl.step(THETA));

    // Gear counter was rolled back, so the retry still carries the coupled rho step
    delayMicroseconds(MIN_STEP_DELAY_US);
    TEST_ASSERT_TRUE(motorControl.step(THETA));
    TEST_ASSERT_FALSE(motorControl.step(RHO));
}

void setUp()
{
    motorControl = MotorControlA4988();
}

void tearDown() {}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_enable);
    RUN_TEST(test_enabled_on_step);
    RUN_TEST(test_disable);
    RUN_TEST(test_step_min_time);
    RUN_TEST(test_theta_coupled_rho_step);
    RUN_TEST(test_theta_waits_for_coupled_rho);
    return UNITY_END();
}