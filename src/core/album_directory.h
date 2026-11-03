#pragma once

#include <filesystem>

#include "image_directory.h"

// Photos to search.
class AlbumDirectory : public ImageDirectory {
public:
    // Same arguments as ImageDirectory.
    AlbumDirectory(std::filesystem::path directory, int max_dimension = 0);
};
