#include <unity.h>
#include "motor_control.h"

MotorControl motorControl;

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

    set_time_ms(1);

    const auto start_time = millis();
    motorControl.step(THETA);
    const auto first_step_time = millis();
    TEST_ASSERT_EQUAL(1, first_step_time);
    motorControl.step(THETA);
    const auto second_step_time = millis();
    // MIN_STEP_DELAY_MS = 5;
    TEST_ASSERT_EQUAL(first_step_time + 5, second_step_time);
}

void setUp()
{
    motorControl = MotorControl();
}

void tearDown() {}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_enable);
    RUN_TEST(test_enabled_on_step);
    RUN_TEST(test_disable);
    RUN_TEST(test_step_min_time);
    return UNITY_END();
}