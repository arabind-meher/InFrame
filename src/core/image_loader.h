#pragma once

#include <filesystem>
#include <opencv2/core.hpp>

// Loads an image (EXIF-rotated); returns an empty Mat if it can't be read.
cv::Mat load_image(const std::filesystem::path& file);

// Returns a copy shrunk so its longest side is at most max_side; unchanged if already smaller.
cv::Mat shrink_image(const cv::Mat& image, int max_side);