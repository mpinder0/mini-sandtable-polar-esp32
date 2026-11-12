// Runs on the ESP32: pio test -e esp32-a4988 (or esp32-uln2803)
// Needs a LittleFS partition that mounts, e.g. after: pio run -t uploadfs
#include <Arduino.h>
#include <LittleFS.h>
#include <unity.h>
#include <constants.h>
#include <Pattern/pattern_file.h>

static const char *TEST_FILE = "/test_pattern_tmp.thr";
static const char *EXAMPLE_FILE = "/patterns/example_rose.thr";

static void write_test_file(const char *content)
{
    File file = LittleFS.open(TEST_FILE, "w");
    TEST_ASSERT_TRUE_MESSAGE(file, "could not create test file");
    file.print(content);
    file.close();
}

void setUp() {}

void tearDown()
{
    if (LittleFS.exists(TEST_FILE))
        LittleFS.remove(TEST_FILE);
}

void test_loads_points()
{
    write_test_file("# test\r\n0 0\r\n1.0 0.5\r\n");
    PatternFile pattern_file(LittleFS, TEST_FILE);
    TEST_ASSERT_TRUE(pattern_file.is_valid());
    std::vector<Coordinates> pattern = pattern_file.get_pattern();
    TEST_ASSERT_EQUAL(2, pattern.size());
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 1.0f, pattern[1].theta);
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, AXIS_MIN_R + 0.5f * (AXIS_MAX_R - AXIS_MIN_R), pattern[1].rho);
}

void test_missing_file_is_invalid()
{
    PatternFile pattern_file(LittleFS, "/does_not_exist.thr");
    TEST_ASSERT_FALSE(pattern_file.is_valid());
}

void test_skips_malformed_lines()
{
    write_test_file("0 0\nnot a point\n1 1\n");
    PatternFile pattern_file(LittleFS, TEST_FILE);
    TEST_ASSERT_TRUE(pattern_file.is_valid());
    TEST_ASSERT_EQUAL(2, pattern_file.get_pattern().size());
    TEST_ASSERT_EQUAL(1, pattern_file.error_count());
}

void test_rejects_file_over_cap()
{
    write_test_file("0 0\n1 0\n2 0\n");
    PatternFile pattern_file(LittleFS, TEST_FILE, 2);
    TEST_ASSERT_FALSE(pattern_file.is_valid());
}

void test_reads_across_chunks()
{
    // longer than PatternFile's 256 byte read chunk, so lines span chunk boundaries
    String content;
    for (int i = 0; i < 100; ++i)
        content += String(i * 0.01f, 5) + " 0.50000\n";
    write_test_file(content.c_str());
    PatternFile pattern_file(LittleFS, TEST_FILE);
    TEST_ASSERT_EQUAL(100, pattern_file.get_pattern().size());
    TEST_ASSERT_EQUAL(0, pattern_file.error_count());
}

void test_example_pattern()
{
    if (!LittleFS.exists(EXAMPLE_FILE))
        TEST_IGNORE_MESSAGE("example pattern not uploaded, run: pio run -t uploadfs");
    PatternFile pattern_file(LittleFS, EXAMPLE_FILE);
    TEST_ASSERT_TRUE(pattern_file.is_valid());
    TEST_ASSERT_EQUAL(0, pattern_file.error_count());
}

void setup()
{
    delay(2000); // allow the serial monitor to connect

    UNITY_BEGIN();
    // don't format on failure, that would wipe uploaded patterns
    if (!LittleFS.begin(false))
    {
        TEST_FAIL_MESSAGE("LittleFS mount failed, run: pio run -t uploadfs");
    }
    else
    {
        RUN_TEST(test_loads_points);
        RUN_TEST(test_missing_file_is_invalid);
        RUN_TEST(test_skips_malformed_lines);
        RUN_TEST(test_rejects_file_over_cap);
        RUN_TEST(test_reads_across_chunks);
        RUN_TEST(test_example_pattern);
        LittleFS.end();
    }
    UNITY_END();
}

void loop() {}
