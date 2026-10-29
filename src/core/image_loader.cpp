#include "image_loader.h"

#include <algorithm>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

cv::Mat load_image(const std::filesystem::path &file) {
    return cv::imread(file.string(), cv::IMREAD_COLOR);
}

cv::Mat shrink_image(const cv::Mat &image, int max_side) {
    int longest = std::max(image.cols, image.rows);

    if (longest <= max_side) { return image; }

    double scale = static_cast<double>(max_side) / longest;
    cv::Mat result;
    cv::resize(image, result, cv::Size(), scale, scale, cv::INTER_AREA);
    return result;
}
