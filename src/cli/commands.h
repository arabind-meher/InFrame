#pragma once

#include <filesystem>

// Settings for the find command, filled in by the CLI.
struct FindOptions {
    std::filesystem::path album;
    std::filesystem::path reference;
    // Base folder; run_find adds a date-time subfolder.
    std::filesystem::path output;
    // Photos are shrunk to this size (0 = full size).
    int max_dimension;
};

// Finds the person in the album and copies matches; returns the exit code.
int run_find(const FindOptions& options);
