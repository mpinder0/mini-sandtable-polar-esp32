#ifndef ABSTRACT_PATTERN_H
#define ABSTRACT_PATTERN_H

#include <vector>
#include <structs.h>

class AbstractPattern
{
public:
    virtual ~AbstractPattern() = default;

    // Returns a vector of coordinates representing the pattern to be drawn.
    virtual std::vector<Coordinates> get_pattern() const = 0;
};

#endif // ABSTRACT_PATTERN_H
