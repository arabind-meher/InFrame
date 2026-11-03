#pragma once

#include <filesystem>
#include <opencv2/core.hpp>

// Loads an image, shrunk to max_dimension (0 = full size). Empty if unreadable.
cv::Mat load_image(const std::filesystem::path& file, int max_dimension = 0);

// Shrinks image so neither side exceeds max_dimension.
cv::Mat shrink_image(const cv::Mat& image, int max_dimension);
