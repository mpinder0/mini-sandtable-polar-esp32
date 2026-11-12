#ifndef UNIT_TEST

#include "pattern_file.h"
#include "thr_parser.h"
#include "constants.h"

PatternFile::PatternFile(fs::FS &fs, const char *path, size_t max_points)
{
    File file = fs.open(path, "r");
    if (!file || file.isDirectory())
    {
        printf("Pattern file %s: could not be opened\n", path);
        return;
    }

    ThrPatternBuilder builder(AXIS_MIN_R, AXIS_MAX_R, max_points);
    char chunk[256];
    while (file.available())
    {
        const size_t length = file.read(reinterpret_cast<uint8_t *>(chunk), sizeof(chunk));
        if (length == 0 || !builder.feed(chunk, length))
            break;
    }
    file.close();

    pattern = builder.finish();
    errors = builder.error_count();

    if (builder.too_many_points())
    {
        printf("Pattern file %s: rejected, more than %u points\n", path, static_cast<unsigned>(max_points));
        return;
    }
    if (errors > 0)
    {
        printf("Pattern file %s: skipped %u malformed line(s), first at line %u\n",
               path, static_cast<unsigned>(errors), static_cast<unsigned>(builder.first_error_line()));
    }
    if (pattern.empty())
    {
        printf("Pattern file %s: no points found\n", path);
        return;
    }
    printf("Pattern file %s: loaded %u points\n", path, static_cast<unsigned>(pattern.size()));
}

std::vector<Coordinates> PatternFile::get_pattern() const
{
    return pattern;
}

#endif // UNIT_TEST
