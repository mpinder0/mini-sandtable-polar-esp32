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

void setUp() {}

void tearDown() {}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_enable);
    RUN_TEST(test_enabled_on_step);
    return UNITY_END();
}