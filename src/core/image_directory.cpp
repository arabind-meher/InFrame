#include "image_directory.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <utility>

// Supported image extensions (lowercase).
static const std::array<std::string, 4> image_extensions = {".jpg", ".jpeg", ".png", ".bmp"};

ImageDirectory::ImageDirectory(std::filesystem::path directory, int max_dimension)
    : directory_path_(std::move(directory)), max_dimension_(max_dimension) {}

bool ImageDirectory::exists() const { return std::filesystem::is_directory(directory_path_); }

std::vector<std::filesystem::path> ImageDirectory::find_images() const {
    std::vector<std::filesystem::path> image_paths;

    // Skip folders we are not allowed to open.
    constexpr auto options = std::filesystem::directory_options::skip_permission_denied;
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(directory_path_, options)) {
        if (entry.is_regular_file() && is_image(entry.path())) {
            image_paths.push_back(entry.path());
        }
    }

    // Same order on every run.
    std::ranges::sort(image_paths);
    return image_paths;
}

cv::Mat ImageDirectory::load_image(const std::filesystem::path& image_path) const {
    cv::Mat image = cv::imread(image_path.string(), cv::IMREAD_COLOR);

    // 0 means keep full size.
    if (max_dimension_ <= 0) return image;

    return shrink_image(image);
}

std::filesystem::path ImageDirectory::directory_path() const { return directory_path_; }

bool ImageDirectory::is_image(const std::filesystem::path& file) {
    // Lowercase, so ".JPG" matches ".jpg".
    std::string ext = file.extension().string();
    for (char& c : ext) {
        c = static_cast<char>(std::tolower(c));
    }

    for (const std::string& image_ext : image_extensions) {
        if (ext == image_ext) {
            return true;
        }
    }
    return false;
}

cv::Mat ImageDirectory::shrink_image(const cv::Mat& image) const {
    int longest = std::max(image.cols, image.rows);

    // Already small enough.
    if (longest <= max_dimension_) {
        return image;
    }

    // INTER_AREA gives the best quality when shrinking.
    double scale = static_cast<double>(max_dimension_) / longest;
    cv::Mat result;
    cv::resize(image, result, cv::Size(), scale, scale, cv::INTER_AREA);
    return result;
}