#include "thr_parser.h"
#include <cmath>
#include <cstdlib>
#include <position_helper.h>

static const char *skip_whitespace(const char *p)
{
    while (*p == ' ' || *p == '\t')
        ++p;
    return p;
}

static bool is_line_end(const char *p)
{
    return *p == '\0' || *p == '\r' || *p == '\n' || *p == '#';
}

ThrLineResult parse_thr_line(const char *line, float min_rho, float max_rho, Coordinates &out)
{
    const char *p = skip_whitespace(line);
    if (is_line_end(p))
        return ThrLineResult::SKIP;

    char *end = nullptr;
    const float theta = std::strtof(p, &end);
    // a number must be consumed and be followed by a separator
    if (end == p || (*end != ' ' && *end != '\t'))
        return ThrLineResult::ERROR;

    p = skip_whitespace(end);
    const float rho = std::strtof(p, &end);
    if (end == p)
        return ThrLineResult::ERROR;

    // only whitespace or a trailing comment allowed after the rho value
    if (!is_line_end(skip_whitespace(end)))
        return ThrLineResult::ERROR;

    if (!std::isfinite(theta) || !std::isfinite(rho))
        return ThrLineResult::ERROR;

    const float rho_clamped = rho < 0.0f ? 0.0f : (rho > 1.0f ? 1.0f : rho);
    out.theta = theta;
    out.rho = min_rho + rho_clamped * (max_rho - min_rho);
    return ThrLineResult::POINT;
}

void normalise_theta_start(std::vector<Coordinates> &pattern)
{
    if (pattern.empty())
        return;

    const float full_rotation = 2.0f * static_cast<float>(PI);
    const float offset = std::floor(pattern[0].theta / full_rotation) * full_rotation;
    if (offset == 0.0f)
        return;

    for (Coordinates &point : pattern)
        point.theta -= offset;
}

ThrPatternBuilder::ThrPatternBuilder(float min_rho, float max_rho, size_t max_points, bool count_only)
    : min_rho(min_rho), max_rho(max_rho), max_points(max_points), count_only(count_only)
{
}

bool ThrPatternBuilder::feed(const char *data, size_t length)
{
    for (size_t i = 0; i < length && !over_cap; ++i)
    {
        const char c = data[i];
        if (c == '\n')
        {
            end_line();
        }
        else if (line_length < MAX_LINE_LENGTH)
        {
            line_buffer[line_length++] = c;
        }
        else
        {
            line_overflow = true;
        }
    }
    return !over_cap;
}

std::vector<Coordinates> ThrPatternBuilder::finish()
{
    if (!over_cap && (line_length > 0 || line_overflow))
        end_line();

    if (over_cap)
    {
        points.clear();
        points.shrink_to_fit();
    }

    normalise_theta_start(points);
    std::vector<Coordinates> result;
    result.swap(points);
    return result;
}

void ThrPatternBuilder::end_line()
{
    ++line_count;

    if (line_overflow)
    {
        record_error();
    }
    else
    {
        line_buffer[line_length] = '\0';
        Coordinates point;
        switch (parse_thr_line(line_buffer, min_rho, max_rho, point))
        {
        case ThrLineResult::POINT:
            if (points_found >= max_points)
            {
                over_cap = true;
            }
            else
            {
                ++points_found;
                if (!count_only)
                    points.push_back(point);
            }
            break;
        case ThrLineResult::ERROR:
            record_error();
            break;
        case ThrLineResult::SKIP:
            break;
        }
    }

    line_length = 0;
    line_overflow = false;
}

void ThrPatternBuilder::record_error()
{
    ++errors;
    if (first_error == 0)
        first_error = line_count;
}
