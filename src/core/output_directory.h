#pragma once

#include <filesystem>

// Where matched photos are copied. Never deletes anything.
class OutputDirectory {
public:
    OutputDirectory(std::filesystem::path directory);

    // Creates the directory once before copying. False if it can't be made.
    bool create() const;

    // Copies an image in; skips it if already there, renames on name clashes.
    bool copy(const std::filesystem::path& image_path) const;

    // The directory's path, e.g. for error messages.
    std::filesystem::path directory_path() const;

private:
    std::filesystem::path directory_path_;
};
