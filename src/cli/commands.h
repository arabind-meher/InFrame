#pragma once

#include <filesystem>

#include "photo_finder.h"

// Settings for the find command, filled in by the CLI.
struct FindOptions {
    std::filesystem::path album;
    std::filesystem::path reference;
    // Base folder; run_find adds a date-time subfolder.
    std::filesystem::path output;
    // Photos are shrunk to this size (0 = full size).
    int max_dimension;
    // Model files and match threshold.
    ModelSettings models;
    // Copy matches into a new output folder (otherwise only list them).
    bool copy = false;
};

// Settings for the scan command: no references, nothing is written.
struct ScanOptions {
    std::filesystem::path album;
    // Photos are shrunk to this size (0 = full size).
    int max_dimension;
    // Only the detector is really needed; the threshold is unused.
    ModelSettings models;
};

// Finds the person in the album and copies matches; returns the exit code.
int run_find(const FindOptions& options);

// Lists face counts and tags for every album photo; returns the exit code.
int run_scan(const ScanOptions& options);
