#include "config.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string_view>

#include "console.h"

// Project root, passed in by CMake (INFRAME_ROOT); backslashes, so joined paths match.
static std::filesystem::path project_root() {
    return std::filesystem::path(INFRAME_ROOT).make_preferred();
}

// Error that names the config file (display_path: UTF-8, never throws on unusual names).
static std::runtime_error config_error(const std::filesystem::path& file, const std::string& msg) {
    return std::runtime_error("Config file " + display_path(file) + ": " + msg);
}

// JSON text is UTF-8; u8string keeps non-English folder names correct on Windows.
static std::filesystem::path utf8_path(const std::string& text) {
    std::u8string utf8(text.begin(), text.end());
    return std::filesystem::path(utf8);
}

// Relative paths start at the project root; absolute paths stay as they are.
static std::filesystem::path root_path(const std::string& text) {
    std::filesystem::path path = utf8_path(text);
    if (path.is_relative()) return project_root() / path;
    return path;
}

// Reads a required, non-empty text value.
static std::string read_string(const nlohmann::json& json, const std::string& key,
                               const std::filesystem::path& file) {
    if (!json.contains(key)) throw config_error(file, "missing key: " + key);

    const nlohmann::json& value = json.at(key);
    if (!value.is_string()) throw config_error(file, key + " must be text");

    std::string text = value.get<std::string>();
    if (text.empty()) throw config_error(file, key + " must not be empty");

    return text;
}

// Reads a required whole number.
static int read_int(const nlohmann::json& json, const std::string& key,
                    const std::filesystem::path& file) {
    if (!json.contains(key)) throw config_error(file, "missing key: " + key);

    const nlohmann::json& value = json.at(key);
    if (!value.is_number_integer()) throw config_error(file, key + " must be an integer");

    return value.get<int>();
}

// Reads a required number (whole or decimal).
static double read_double(const nlohmann::json& json, const std::string& key,
                          const std::filesystem::path& file) {
    if (!json.contains(key)) throw config_error(file, "missing key: " + key);

    const nlohmann::json& value = json.at(key);
    if (!value.is_number()) throw config_error(file, key + " must be a number");

    return value.get<double>();
}

// Every key the config may contain; anything else is reported as unknown.
static constexpr std::array<std::string_view, 6> kConfigKeys = {
    "output",         "max_dimension",    "model_directory",
    "detector_model", "recognizer_model", "match_threshold"};

std::filesystem::path config_path() { return project_root() / "config.json"; }

Config load_config(const std::filesystem::path& path) {
    Config config;

    // Checked first so the user gets "not found" instead of a parse error.
    if (!std::filesystem::exists(path))
        throw config_error(path, "not found (copy config.example.json to config.json)");

    // Parse errors already give line and column; add the file name.
    std::ifstream file(path);
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(file);
    } catch (const nlohmann::json::exception& e) {
        throw config_error(path, e.what());
    }

    // Valid JSON like [1, 2] is still not a config.
    if (!json.is_object()) throw config_error(path, "must be a JSON object { ... }");

    // Collect unknown keys (e.g. typos); main prints them as warnings.
    for (const auto& item : json.items()) {
        if (std::ranges::find(kConfigKeys, item.key()) == kConfigKeys.end()) {
            config.unknown_keys.push_back(item.key());
        }
    }

    // Each read returns a valid value or throws on the first problem.
    config.output_directory = root_path(read_string(json, "output", path));
    config.max_dimension = read_int(json, "max_dimension", path);
    config.model_directory = root_path(read_string(json, "model_directory", path));
    config.detector_model = read_string(json, "detector_model", path);
    config.recognizer_model = read_string(json, "recognizer_model", path);
    config.match_threshold = read_double(json, "match_threshold", path);

    // Ranges: 0 means full size; below 320 faces get too small for detection.
    if (config.max_dimension != 0 && (config.max_dimension < 320 || config.max_dimension > 4096))
        throw config_error(path, "max_dimension must be 0 or 320-4096");

    if (config.match_threshold < 0.0 || config.match_threshold > 1.0)
        throw config_error(path, "match_threshold must be between 0 and 1");

    return config;
}
