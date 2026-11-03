#pragma once

#include <filesystem>
#include <opencv2/core.hpp>
#include <vector>

// A directory of photos: finds and loads its images.
class ImageDirectory {
public:
    // max_dimension: longest side after loading (0 = full size).
    ImageDirectory(std::filesystem::path directory, int max_dimension = 0);

    // True if the directory exists.
    bool exists() const;

    // All images in the directory and its subdirectories, sorted.
    std::vector<std::filesystem::path> find_images() const;

    // Loads and shrinks an image. Empty if unreadable.
    cv::Mat load_image(const std::filesystem::path& image_path) const;

    // The directory's path, e.g. for error messages.
    std::filesystem::path directory_path() const;

protected:
    std::filesystem::path directory_path_;

private:
    int max_dimension_;

    // True if the file has an image extension.
    static bool is_image(const std::filesystem::path& file);

    // Shrinks image so neither side exceeds max_dimension_.
    cv::Mat shrink_image(const cv::Mat& image) const;
};
