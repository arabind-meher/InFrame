#pragma once

#include "face_detector.h"

#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>


// Turns faces into embeddings with the SFace model and compares them.
class FaceRecognizer {
public:
    // Loads the SFace model.
    FaceRecognizer(const std::filesystem::path &model_path);

    // Returns the 128-number embedding of a face detected in image.
    cv::Mat embed(const cv::Mat &image, const Face &face);

    // Cosine similarity: ~1.0 same face, above ~0.36 likely the same person.
    double similarity(const cv::Mat &embedding1, const cv::Mat &embedding2);

private:
    // Loaded once, reused for every face.
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
};
