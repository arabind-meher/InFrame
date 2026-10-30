#pragma once

#include <filesystem>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

// One detected face: its box and the full YuNet row (box, landmarks, score).
struct Face {
    cv::Rect box;
    cv::Mat detection;
};

// Finds faces in images with the YuNet model.
class FaceDetector {
public:
    // Loads the YuNet model.
    FaceDetector(const std::filesystem::path &model_path);

    // Returns every face found in image.
    std::vector<Face> detect(const cv::Mat &image);

private:
    // Loaded once, reused for every image.
    cv::Ptr<cv::FaceDetectorYN> detector_;
};
