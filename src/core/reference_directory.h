#pragma once

#include <filesystem>
#include <utility>

#include "image_directory.h"

// Photos of the person to find.
class ReferenceDirectory : public ImageDirectory {
public:
    // Same arguments as ImageDirectory.
    ReferenceDirectory(std::filesystem::path directory, int max_dimension = 0)
        : ImageDirectory(std::move(directory), max_dimension) {};
};