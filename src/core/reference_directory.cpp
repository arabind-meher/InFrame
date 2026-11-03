#include "reference_directory.h"

#include <utility>

// The parent stores the path and max dimension.
ReferenceDirectory::ReferenceDirectory(std::filesystem::path directory, int max_dimension)
    : ImageDirectory(std::move(directory), max_dimension) {}