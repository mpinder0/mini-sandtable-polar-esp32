#ifndef UNIT_TEST

#include <Arduino.h>
#include "pattern_file.h"
#include "thr_parser.h"
#include "constants.h"

// Feed the whole file through the builder. Returns false if the file can't be opened.
static bool read_into(fs::FS &fs, const char *path, ThrPatternBuilder &builder)
{
    File file = fs.open(path, "r");
    if (!file || file.isDirectory())
        return false;

    char chunk[256];
    while (file.available())
    {
        const size_t length = file.read(reinterpret_cast<uint8_t *>(chunk), sizeof(chunk));
        if (length == 0 || !builder.feed(chunk, length))
            break;
    }
    file.close();
    return true;
}

PatternFile::PatternFile(fs::FS &fs, const char *path, size_t max_points)
{
    // Pass 1: count points without storing them
    ThrPatternBuilder counter(AXIS_MIN_R, AXIS_MAX_R, max_points, true);
    if (!read_into(fs, path, counter))
    {
        printf("Pattern file %s: could not be opened\n", path);
        return;
    }
    counter.finish();

    if (counter.too_many_points())
    {
        printf("Pattern file %s: rejected, more than %u points\n", path, static_cast<unsigned>(max_points));
        return;
    }
    const size_t point_count = counter.point_count();
    if (point_count == 0)
    {
        printf("Pattern file %s: no points found\n", path);
        return;
    }

    // The pattern needs one contiguous block, check it will fit before allocating
    const size_t bytes_needed = point_count * sizeof(Coordinates);
    const size_t largest_block = ESP.getMaxAllocHeap();
    if (bytes_needed + HEAP_MARGIN_BYTES > largest_block)
    {
        printf("Pattern file %s: rejected, %u points need %u bytes, largest free heap block is %u bytes\n",
               path, static_cast<unsigned>(point_count), static_cast<unsigned>(bytes_needed),
               static_cast<unsigned>(largest_block));
        return;
    }

    // Pass 2: load into storage allocated once at the exact size
    ThrPatternBuilder builder(AXIS_MIN_R, AXIS_MAX_R, max_points);
    builder.reserve(point_count);
    if (!read_into(fs, path, builder))
    {
        printf("Pattern file %s: could not be re-opened\n", path);
        return;
    }
    pattern = builder.finish();
    errors = builder.error_count();

    if (errors > 0)
    {
        printf("Pattern file %s: skipped %u malformed line(s), first at line %u\n",
               path, static_cast<unsigned>(errors), static_cast<unsigned>(builder.first_error_line()));
    }
    printf("Pattern file %s: loaded %u points (%u bytes)\n", path,
           static_cast<unsigned>(pattern.size()), static_cast<unsigned>(pattern.size() * sizeof(Coordinates)));
}

std::vector<Coordinates> PatternFile::get_pattern() const
{
    return pattern;
}

std::vector<Coordinates> PatternFile::take_pattern()
{
    std::vector<Coordinates> taken;
    taken.swap(pattern);
    return taken;
}

#endif // UNIT_TEST
