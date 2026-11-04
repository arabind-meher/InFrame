#pragma once

#include <filesystem>

// Settings for the find command, filled in by the CLI.
struct FindOptions {
    std::filesystem::path album;
    std::filesystem::path reference;
    std::filesystem::path output = "InFrame Output";
};

// Finds the person in the album and copies matches; returns the exit code.
int run_find(const FindOptions& options);
