#ifndef THR_PARSER_H
#define THR_PARSER_H

#include <cstddef>
#include <vector>
#include <structs.h>

// Parsing for Sisyphus style theta-rho (.thr) pattern files.
//
// Format: one point per line, "<theta> <rho>" separated by spaces or tabs.
//   theta - radians, unwrapped (can run past 2pi for multiple rotations)
//   rho   - normalised 0.0 (centre) to 1.0 (edge)
// Lines starting with '#' are comments, blank lines are ignored.
//
// No filesystem or Arduino dependencies, so it can be unit tested on the native env.

enum class ThrLineResult
{
    POINT, // line held a valid point, written to out
    SKIP,  // blank or comment line
    ERROR  // malformed line
};

// Parse a single line (no newline required, trailing '\r' tolerated).
// Rho is clamped to 0..1 and scaled to min_rho..max_rho (mm).
ThrLineResult parse_thr_line(const char *line, float min_rho, float max_rho, Coordinates &out);

// Shift every theta by a whole number of rotations so the first point lies in [0, 2pi).
// The shape is unchanged, it avoids winding the table through extra rotations to reach the start.
void normalise_theta_start(std::vector<Coordinates> &pattern);

// Builds a pattern from .thr text fed in arbitrary sized chunks (e.g. as read from a file).
// Handles line splitting, over-long lines and a cap on the number of points.
class ThrPatternBuilder
{
public:
    // Longest line accepted, longer lines are counted as errors
    static constexpr size_t MAX_LINE_LENGTH = 96;

    ThrPatternBuilder(float min_rho, float max_rho, size_t max_points);

    // Feed the next chunk of text. Returns false once the point cap has been exceeded,
    // after which further input is ignored.
    bool feed(const char *data, size_t length);

    // Process any final line without a trailing newline, normalise theta and return the pattern.
    // Returns an empty pattern if the point cap was exceeded.
    std::vector<Coordinates> finish();

    size_t lines_read() const { return line_count; }
    size_t error_count() const { return errors; }
    // Line number (1 based) of the first malformed line, 0 if none
    size_t first_error_line() const { return first_error; }
    bool too_many_points() const { return over_cap; }

private:
    float min_rho;
    float max_rho;
    size_t max_points;

    std::vector<Coordinates> points;
    char line_buffer[MAX_LINE_LENGTH + 1];
    size_t line_length = 0;
    bool line_overflow = false;

    size_t line_count = 0;
    size_t errors = 0;
    size_t first_error = 0;
    bool over_cap = false;

    void end_line();
    void record_error();
};

#endif // THR_PARSER_H
