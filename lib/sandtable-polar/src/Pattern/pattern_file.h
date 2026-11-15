#ifndef PATTERN_FILE_H
#define PATTERN_FILE_H

// ESP32 only, the parsing it relies on (thr_parser.h) is unit tested on the native env
#ifndef UNIT_TEST

#include <FS.h>
#include <vector>
#include "abstract_pattern.h"

// A pattern loaded from a Sisyphus style theta-rho (.thr) file, see thr_parser.h for the format.
// The whole pattern is held in memory (8 bytes per point). The file is read twice: first to count
// the points, then to load them into storage allocated once at the exact size. Files are rejected
// if they exceed max_points or won't fit in the largest free heap block.
// The filesystem must already be mounted (e.g. LittleFS.begin()).
class PatternFile : public AbstractPattern
{
public:
    // Upper bound on points, 160 KB. In practice the free heap check is usually the tighter limit.
    static constexpr size_t DEFAULT_MAX_POINTS = 20000;
    // Heap left free in the largest block after loading, for other allocations
    static constexpr size_t HEAP_MARGIN_BYTES = 16 * 1024;

    PatternFile(fs::FS &fs, const char *path, size_t max_points = DEFAULT_MAX_POINTS);

    // Paths of every file in dir ending in extension (case insensitive), sorted by name.
    // Only the paths are returned, so patterns can be loaded one at a time.
    static std::vector<String> list(fs::FS &fs, const char *dir, const char *extension = ".thr");
    // Load a file straight into a vector, empty if it couldn't be loaded.
    // Nothing else is kept, the points are freed when the returned vector goes out of scope.
    static std::vector<Coordinates> load(fs::FS &fs, const char *path, size_t max_points = DEFAULT_MAX_POINTS);

    // Returns a copy of the pattern. For large patterns prefer take_pattern().
    std::vector<Coordinates> get_pattern() const override;
    // Moves the pattern out without copying, the PatternFile is left empty (is_valid() false).
    std::vector<Coordinates> take_pattern();

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
