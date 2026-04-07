#include "abstract_pattern.h"
#include "position_helper.h"

class PatternSpiral : public AbstractPattern
{
public:
    PatternSpiral(bool direction_outward = true);
    std::vector<Coordinates> get_pattern() const override;

private:
    std::vector<Coordinates> pattern;
};

PatternSpiral::PatternSpiral(bool direction_outward)
{
    // Generate a simple spiral pattern
    const float min_radius = 0.0f;  // mm
    const float max_radius = 70.0f; // mm

    const float sprial_rate = 5.0f;                                          // mm moved in theta per full revolution
    const int points_per_revolution = 10;                                    // Number of points per full revolution
    const float spiral_rate_per_point = sprial_rate / points_per_revolution; // mm per point
    const float angle_step = (2 * PI) / points_per_revolution;               // radians per point

    if (direction_outward)
    {
        // Outward spiral: start from the center and move outward
        for (int i = 0; i < 1000; ++i) // Arbitrary large number to ensure we fill the pattern
        {
            float t = i * angle_step;                           // angle in radians
            float r = min_radius + (i * spiral_rate_per_point); // radius increases with each point
            if (r > max_radius)
                break;
            pattern.push_back({t, r});
        }
    }
    else
    {
        // Inward spiral: start from the outer edge and move inward
        for (int i = 0; i < 1000; ++i) // Arbitrary large number to ensure we fill the pattern
        {
            float t = i * angle_step;                           // angle in radians
            float r = max_radius - (i * spiral_rate_per_point); // radius decreases with each point
            if (r < min_radius)
                break;
            pattern.push_back({t, r});
        }
    }
}

std::vector<Coordinates> PatternSpiral::get_pattern() const
{
    return pattern;
}