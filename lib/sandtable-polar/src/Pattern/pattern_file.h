#ifndef PATTERN_FILE_H
#define PATTERN_FILE_H

// ESP32 only, the parsing it relies on (thr_parser.h) is unit tested on the native env
#ifndef UNIT_TEST

#include <FS.h>
#include "abstract_pattern.h"

// A pattern loaded from a Sisyphus style theta-rho (.thr) file, see thr_parser.h for the format.
// The whole file is loaded into memory, so files are capped at max_points.
// The filesystem must already be mounted (e.g. LittleFS.begin()).
class PatternFile : public AbstractPattern
{
public:
    // 8 bytes per point, the default cap keeps a loaded pattern around 40 KB
    static constexpr size_t DEFAULT_MAX_POINTS = 5000;

    PatternFile(fs::FS &fs, const char *path, size_t max_points = DEFAULT_MAX_POINTS);
    std::vector<Coordinates> get_pattern() const override;

    // True if the file was read and held at least one point
    bool is_valid() const { return !pattern.empty(); }
    // Malformed lines that were skipped
    size_t error_count() const { return errors; }

private:
    std::vector<Coordinates> pattern;
    size_t errors = 0;
};

#endif // UNIT_TEST
#endif // PATTERN_FILE_H
