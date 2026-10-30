#include "face_recognizer.h"


FaceRecognizer::FaceRecognizer(const std::filesystem::path &model_path)
    : recognizer_(cv::FaceRecognizerSF::create(model_path.string(), "")) {
}

cv::Mat FaceRecognizer::embed(const cv::Mat &image, const Face &face) {
    cv::Mat aligned;
    recognizer_->alignCrop(image, face.detection, aligned);

    cv::Mat embedding;
    recognizer_->feature(aligned, embedding);

    return embedding.clone();
}

double FaceRecognizer::similarity(const cv::Mat &embedding1, const cv::Mat &embedding2) {
    return recognizer_->match(embedding1, embedding2, cv::FaceRecognizerSF::FR_COSINE);
}
