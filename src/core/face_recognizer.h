#pragma once

#include "face_detector.h"

#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>


class FaceRecognizer {
public:
    FaceRecognizer(const std::filesystem::path &model_path);

    cv::Mat embed(const cv::Mat &image, const Face &face);

    double similarity(const cv::Mat &embedding1, const cv::Mat &embedding2);

private:
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
};
