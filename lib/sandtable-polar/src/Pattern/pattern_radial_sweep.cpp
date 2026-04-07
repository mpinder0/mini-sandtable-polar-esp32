#include "abstract_pattern.h"
#include "position_helper.h"

class PatternRadialSweep : public AbstractPattern
{
public:
    PatternRadialSweep();
    std::vector<Coordinates> get_pattern() const override;

private:
    std::vector<Coordinates> pattern;
};

PatternRadialSweep::PatternRadialSweep()
{
    // Generate a simple radial sweep pattern
    const float min_radius = 0.0f;                       // mm
    const float max_radius = 70.0f;                      // mm
    const int num_points = 100;                          // Total number of rotational points
    const float full_angle_step = (2 * PI) / num_points; // radians per point
    const float half_angle_step = PI / 2;                // radians per point for the return sweep

    for (int i = 0; i < num_points; ++i)
    {
        float t = i * full_angle_step;      // angle in radians
        pattern.push_back({t, min_radius}); // start at the center
        t = t + half_angle_step;            // angle in radians, half step further
        pattern.push_back({t, max_radius}); // Move to the outer edge
    }
}

std::vector<Coordinates> PatternRadialSweep::get_pattern() const
{
    return pattern;
}