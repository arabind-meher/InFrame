#include "face_detector.h"

FaceDetector::FaceDetector(const std::filesystem::path& model_path)
    : detector_(cv::FaceDetectorYN::create(model_path.string(), "", cv::Size(320, 320))) {}

std::vector<Face> FaceDetector::detect(const cv::Mat& image) {
    cv::Mat faces;
    detector_->setInputSize(image.size());
    detector_->detect(image, faces);

    // One row per face: x, y, width, height, 5 landmarks, score.
    std::vector<Face> result;
    for (int i = 0; i < faces.rows; i++) {
        int x = static_cast<int>(faces.at<float>(i, 0));
        int y = static_cast<int>(faces.at<float>(i, 1));
        int width = static_cast<int>(faces.at<float>(i, 2));
        int height = static_cast<int>(faces.at<float>(i, 3));

        result.push_back(Face{cv::Rect(x, y, width, height), faces.row(i).clone()});
    }

    return result;
}
