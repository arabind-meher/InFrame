#include "face_recognizer.h"

#include <algorithm>
#include <vector>

FaceRecognizer::FaceRecognizer(const std::filesystem::path& model_path)
    : recognizer_(cv::FaceRecognizerSF::create(model_path.string(), "")) {}

cv::Mat FaceRecognizer::embed(const cv::Mat& image, const Face& face) {
    // Straighten and crop the face using its landmarks.
    cv::Mat aligned;
    recognizer_->alignCrop(image, face.detection, aligned);

    cv::Mat embedding;
    recognizer_->feature(aligned, embedding);

    // Copy, because feature() may reuse its output buffer.
    return embedding.clone();
}

double FaceRecognizer::similarity(const cv::Mat& embedding1, const cv::Mat& embedding2) {
    return recognizer_->match(embedding1, embedding2, cv::FaceRecognizerSF::FR_COSINE);
}

double FaceRecognizer::best_similarity(const cv::Mat& face_embedding,
                                       const std::vector<cv::Mat>& ref_embeddings) {
    double best_score = -1.0;

    for (const auto& ref_face : ref_embeddings) {
        double score = similarity(face_embedding, ref_face);
        best_score = std::max(best_score, score);
    }

    return best_score;
}
