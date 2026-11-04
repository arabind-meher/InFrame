#include "album_directory.h"

#include <utility>

// The parent stores the path and max dimension.
AlbumDirectory::AlbumDirectory(std::filesystem::path directory, int max_dimension)
    : ImageDirectory(std::move(directory), max_dimension) {}

bool AlbumDirectory::contains(const std::filesystem::path& path) const {
    // Full, clean paths, so "tmp/../tmp/album" and "tmp/album" compare equal.
    std::filesystem::path album = std::filesystem::weakly_canonical(directory_path_);
    std::filesystem::path other = std::filesystem::weakly_canonical(path);

    // Route from the album to the other path, e.g. "matched" or "../refs".
    std::filesystem::path relative = other.lexically_relative(album);

    // Empty: no route at all (e.g. a different drive).
    if (relative.empty()) return false;

    // Starting with "..": the path is outside the album.
    if (*relative.begin() == "..") return false;

    // Anything else is inside; "." is the album itself.
    return true;
}