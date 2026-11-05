#pragma once

#include <filesystem>
#include <string>
#include <vector>

// Settings from config.json; every field comes from the file (no defaults).
struct Config {
    // Base folder; each run copies into a new date-time subfolder.
    std::filesystem::path output_directory;
    // Photos are shrunk to this size (0 = full size).
    int max_dimension;
    std::filesystem::path model_directory;
    // File names inside model_directory.
    std::string detector_model;
    std::string recognizer_model;
    // Lowest similarity score that counts as a match.
    double match_threshold;
    // Keys in the file that aren't settings; main warns about them.
    std::vector<std::string> unknown_keys;
};

// config.json in the project root.
std::filesystem::path config_path();

// Reads and checks the config; throws std::runtime_error naming the file on any problem.
Config load_config(const std::filesystem::path& path);
