#include "image_loader.h"

#include <algorithm>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

cv::Mat load_image(const std::filesystem::path &file, int max_dimension) {
    cv::Mat image = cv::imread(file.string(), cv::IMREAD_COLOR);

    if (max_dimension <= 0)
        return image;

    return shrink_image(image, max_dimension);
}

cv::Mat shrink_image(const cv::Mat &image, int max_dimension) {
    int longest = std::max(image.cols, image.rows);

    if (longest <= max_dimension) { return image; }

    double scale = static_cast<double>(max_dimension) / longest;
    cv::Mat result;
    cv::resize(image, result, cv::Size(), scale, scale, cv::INTER_AREA);
    return result;
}
