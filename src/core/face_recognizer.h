#pragma once

#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

#include "face_detector.h"

// OpenCV's recommended SFace cosine threshold.
inline constexpr double kMatchThreshold = 0.363;

// Turns faces into embeddings with the SFace model and compares them.
class FaceRecognizer {
public:
    // Loads the SFace model.
    FaceRecognizer(const std::filesystem::path& model_path);

    // Returns the 128-number embedding of a face detected in image.
    cv::Mat embed(const cv::Mat& image, const Face& face);

    // Cosine similarity: ~1.0 same face, above ~0.36 likely the same person.
    double similarity(const cv::Mat& embedding1, const cv::Mat& embedding2);

    // Highest similarity between the face and any reference.
    double best_similarity(const cv::Mat& face_embedding,
                           const std::vector<cv::Mat>& ref_embeddings);

private:
    // Loaded once, reused for every face.
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
};
