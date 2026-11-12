#include <unity.h>
#include <cstring>
#include <string>
#include <position_helper.h>
#include <Pattern/thr_parser.h>

static const float TWO_PI_F = 2.0f * PI;
static const float MIN_R = 0.0f;
static const float MAX_R = 66.5f;

static std::vector<Coordinates> build(const char *text, size_t max_points = 100)
{
    ThrPatternBuilder builder(MIN_R, MAX_R, max_points);
    builder.feed(text, std::strlen(text));
    return builder.finish();
}

// --- parse_thr_line ---

void test_parse_point()
{
    Coordinates c;
    TEST_ASSERT_EQUAL(ThrLineResult::POINT, parse_thr_line("1.5 0.5", MIN_R, MAX_R, c));
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.5f, c.theta);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 33.25f, c.rho);
}

void test_parse_tabs_whitespace_and_crlf()
{
    Coordinates c;
    TEST_ASSERT_EQUAL(ThrLineResult::POINT, parse_thr_line("  \t-3.0\t\t1.0  \r", MIN_R, MAX_R, c));
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, -3.0f, c.theta);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, MAX_R, c.rho);
}

void test_parse_trailing_comment()
{
    Coordinates c;
    TEST_ASSERT_EQUAL(ThrLineResult::POINT, parse_thr_line("0.1 0.2 # start", MIN_R, MAX_R, c));
}

void test_parse_scales_to_min_rho()
{
    Coordinates c;
    TEST_ASSERT_EQUAL(ThrLineResult::POINT, parse_thr_line("0 0.5", 10.0f, 20.0f, c));
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 15.0f, c.rho);
}

void test_parse_skips_blank_and_comments()
{
    Coordinates c;
    TEST_ASSERT_EQUAL(ThrLineResult::SKIP, parse_thr_line("", MIN_R, MAX_R, c));
    TEST_ASSERT_EQUAL(ThrLineResult::SKIP, parse_thr_line("   \r", MIN_R, MAX_R, c));
    TEST_ASSERT_EQUAL(ThrLineResult::SKIP, parse_thr_line("# comment", MIN_R, MAX_R, c));
}

void test_parse_errors()
{
    Coordinates c;
    TEST_ASSERT_EQUAL(ThrLineResult::ERROR, parse_thr_line("1.0", MIN_R, MAX_R, c));
    TEST_ASSERT_EQUAL(ThrLineResult::ERROR, parse_thr_line("abc 0.5", MIN_R, MAX_R, c));
    TEST_ASSERT_EQUAL(ThrLineResult::ERROR, parse_thr_line("1.0 xyz", MIN_R, MAX_R, c));
    TEST_ASSERT_EQUAL(ThrLineResult::ERROR, parse_thr_line("1.0,0.5", MIN_R, MAX_R, c));
    TEST_ASSERT_EQUAL(ThrLineResult::ERROR, parse_thr_line("1.0 0.5 2.0", MIN_R, MAX_R, c));
    TEST_ASSERT_EQUAL(ThrLineResult::ERROR, parse_thr_line("nan 0.5", MIN_R, MAX_R, c));
    TEST_ASSERT_EQUAL(ThrLineResult::ERROR, parse_thr_line("1.0 inf", MIN_R, MAX_R, c));
}

void test_parse_clamps_rho()
{
    Coordinates c;
    parse_thr_line("0 1.5", MIN_R, MAX_R, c);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, MAX_R, c.rho);
    parse_thr_line("0 -0.2", MIN_R, MAX_R, c);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, MIN_R, c.rho);
}

// --- normalise_theta_start ---

void test_normalise_removes_whole_rotations()
{
    std::vector<Coordinates> p = {{3 * TWO_PI_F + 0.5f, 1.0f}, {3 * TWO_PI_F + 1.5f, 2.0f}};
    normalise_theta_start(p);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.5f, p[0].theta);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 1.5f, p[1].theta);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 2.0f, p[1].rho);
}

void test_normalise_negative_start()
{
    std::vector<Coordinates> p = {{-0.5f, 0.0f}, {-1.0f, 0.0f}};
    normalise_theta_start(p);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, TWO_PI_F - 0.5f, p[0].theta);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, TWO_PI_F - 1.0f, p[1].theta);
}

void test_normalise_leaves_first_rotation()
{
    std::vector<Coordinates> p = {{1.0f, 0.0f}, {20.0f, 0.0f}};
    normalise_theta_start(p);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.0f, p[0].theta);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 20.0f, p[1].theta);

    std::vector<Coordinates> empty;
    normalise_theta_start(empty);
    TEST_ASSERT_TRUE(empty.empty());
}

// --- ThrPatternBuilder ---

void test_builder_reads_file_text()
{
    std::vector<Coordinates> p = build("# header\r\n0 0\r\n\r\n1.0 0.5\r\n2.0 1.0");
    TEST_ASSERT_EQUAL(3, p.size());
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 2.0f, p[2].theta);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, MAX_R, p[2].rho);
}

void test_builder_split_chunks()
{
    const char *text = "0.25 0.5\n1.75 0.25\n";
    ThrPatternBuilder builder(MIN_R, MAX_R, 100);
    // feed one character at a time, lines span chunk boundaries
    for (size_t i = 0; i < std::strlen(text); ++i)
        builder.feed(text + i, 1);
    std::vector<Coordinates> p = builder.finish();
    TEST_ASSERT_EQUAL(2, p.size());
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.75f, p[1].theta);
    TEST_ASSERT_EQUAL(2, builder.lines_read());
}

void test_builder_counts_errors()
{
    ThrPatternBuilder builder(MIN_R, MAX_R, 100);
    const char *text = "0 0\nbad line\n1 1\n1 2 3\n";
    builder.feed(text, std::strlen(text));
    std::vector<Coordinates> p = builder.finish();
    TEST_ASSERT_EQUAL(2, p.size());
    TEST_ASSERT_EQUAL(2, builder.error_count());
    TEST_ASSERT_EQUAL(2, builder.first_error_line());
}

void test_builder_overlong_line_is_error()
{
    std::string text = "0 0\n";
    text += std::string(ThrPatternBuilder::MAX_LINE_LENGTH + 10, '1');
    text += " 0.5\n1 1\n";
    ThrPatternBuilder builder(MIN_R, MAX_R, 100);
    builder.feed(text.c_str(), text.size());
    std::vector<Coordinates> p = builder.finish();
    TEST_ASSERT_EQUAL(2, p.size());
    TEST_ASSERT_EQUAL(1, builder.error_count());
    TEST_ASSERT_EQUAL(2, builder.first_error_line());
}

void test_builder_rejects_over_cap()
{
    ThrPatternBuilder builder(MIN_R, MAX_R, 2);
    const char *text = "0 0\n1 0\n2 0\n";
    TEST_ASSERT_FALSE(builder.feed(text, std::strlen(text)));
    TEST_ASSERT_TRUE(builder.too_many_points());
    TEST_ASSERT_TRUE(builder.finish().empty());
}

void test_builder_at_cap_is_accepted()
{
    ThrPatternBuilder builder(MIN_R, MAX_R, 2);
    const char *text = "0 0\n1 0\n# trailing comment\n";
    TEST_ASSERT_TRUE(builder.feed(text, std::strlen(text)));
    TEST_ASSERT_FALSE(builder.too_many_points());
    TEST_ASSERT_EQUAL(2, builder.finish().size());
}

void test_builder_normalises_theta()
{
    std::vector<Coordinates> p = build("37.7 0\n38.7 0\n");
    TEST_ASSERT_TRUE(p[0].theta >= 0.0f && p[0].theta < TWO_PI_F);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 1.0f, p[1].theta - p[0].theta);
}

void test_builder_empty_input()
{
    TEST_ASSERT_TRUE(build("").empty());
    TEST_ASSERT_TRUE(build("# only comments\n\n").empty());
}

void setUp() {}
void tearDown() {}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_parse_point);
    RUN_TEST(test_parse_tabs_whitespace_and_crlf);
    RUN_TEST(test_parse_trailing_comment);
    RUN_TEST(test_parse_scales_to_min_rho);
    RUN_TEST(test_parse_skips_blank_and_comments);
    RUN_TEST(test_parse_errors);
    RUN_TEST(test_parse_clamps_rho);
    RUN_TEST(test_normalise_removes_whole_rotations);
    RUN_TEST(test_normalise_negative_start);
    RUN_TEST(test_normalise_leaves_first_rotation);
    RUN_TEST(test_builder_reads_file_text);
    RUN_TEST(test_builder_split_chunks);
    RUN_TEST(test_builder_counts_errors);
    RUN_TEST(test_builder_overlong_line_is_error);
    RUN_TEST(test_builder_rejects_over_cap);
    RUN_TEST(test_builder_at_cap_is_accepted);
    RUN_TEST(test_builder_normalises_theta);
    RUN_TEST(test_builder_empty_input);
    return UNITY_END();
}
