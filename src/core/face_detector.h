#pragma once

#include <filesystem>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

// Detects faces in images using the YuNet model. Load once, reuse for many images.
class FaceDetector {
public:
    // Loads the YuNet .onnx model from model_path.
    FaceDetector(const std::filesystem::path &model_path);

    // Returns one box per face found in image.
    std::vector<cv::Rect> detect(const cv::Mat &image);

private:
    // The loaded model, kept between detect() calls.
    cv::Ptr<cv::FaceDetectorYN> detector_;
};
