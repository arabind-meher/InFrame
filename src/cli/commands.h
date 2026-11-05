#pragma once

#include <filesystem>

// Settings for the find command, filled in by the CLI.
struct FindOptions {
    std::filesystem::path album;
    std::filesystem::path reference;
    // Defaults move to the config in step 4.
    std::filesystem::path output = "InFrame Output";
    int max_dimension = 1280;
};

// Finds the person in the album and copies matches; returns the exit code.
int run_find(const FindOptions& options);
