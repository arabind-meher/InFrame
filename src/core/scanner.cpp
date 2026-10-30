#include "scanner.h"

#include <array>
#include <cctype>
#include <string>


// Accepted image extensions (lowercase).
static const std::array<std::string, 4> image_extensions = {
    ".jpg", ".jpeg", ".png", ".bmp"
};

// True if file has an image extension (any case).
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
