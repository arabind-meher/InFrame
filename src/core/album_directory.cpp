#include "album_directory.h"

#include <utility>

// The parent stores the path and max dimension.
AlbumDirectory::AlbumDirectory(std::filesystem::path directory, int max_dimension)
    : ImageDirectory(std::move(directory), max_dimension) {}
