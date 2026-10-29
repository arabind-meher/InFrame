#include "scanner.h"

#include <array>
#include <cctype>
#include <string>

// Image file extensions the scanner accepts (lowercase)
static const std::array<std::string, 4> image_extensions = {
    ".jpg", ".jpeg", ".png", ".bmp"
};

// True if the file has one of the image extensions, ignoring case.
static bool is_image(const std::filesystem::path &file) {
    std::string ext = file.extension().string();
    for (char &c: ext) {
        c = static_cast<char>(std::tolower(c));
    }

    for (const std::string &image_ext: image_extensions) {
        if (ext == image_ext) {
            return true;
        }
    }
    return false;
}

// Returns all image files in directory and its subfolders
std::vector<std::filesystem::path> find_images(const std::filesystem::path &directory) {
    std::vector<std::filesystem::path> images;

    const auto options = std::filesystem::directory_options::skip_permission_denied;
    for (const auto &entry: std::filesystem::recursive_directory_iterator(directory, options)) {
        if (entry.is_regular_file() && is_image(entry.path())) {
            images.push_back(entry.path());
        }
    }

    return images;
}
