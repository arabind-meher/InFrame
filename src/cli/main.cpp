#include <iostream>

#include "scanner.h"
#include "image_loader.h"
#include "face_detector.h"
#include "face_recognizer.h"

#include <opencv2/core/utils/logger.hpp>
#include <iomanip>


int main(int argc, char *argv[]) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    if (argc != 3) {
        std::cerr << "Usage: inframe <folder> <ref_image>" << std::endl;
        return 2;
    }

    std::filesystem::path directory = argv[1];
    if (!std::filesystem::is_directory(directory)) {
        std::cerr << "Not a Directory: " << directory.string() << std::endl;
        return 1;
    }

    const std::filesystem::path model_path("models");

    std::filesystem::path face_detection_model_path = model_path / "face_detection_yunet_2023mar.onnx";
    if (!std::filesystem::exists(face_detection_model_path)) {
        std::cerr << "Model not found: " << face_detection_model_path.string() << std::endl;
        return 1;
    }

    std::filesystem::path face_recognition_model_path = model_path / "face_recognition_sface_2021dec.onnx";
    if (!std::filesystem::exists(face_recognition_model_path)) {
        std::cerr << "Model not found: " << face_recognition_model_path.string() << std::endl;
        return 1;
    }

    int loaded = 0;
    int skipped = 0;

    FaceDetector detector(face_detection_model_path);
    FaceRecognizer recognizer(face_recognition_model_path);

    std::vector<std::filesystem::path> image_paths = find_images(directory);

    std::filesystem::path ref_image_path(argv[2]);
    if (!std::filesystem::exists(ref_image_path)) {
        std::cerr << "Reference image path not found: " << ref_image_path.string() << std::endl;
        return 1;
    }

    cv::Mat ref_image = load_image(ref_image_path, 1280);
    std::vector<Face> ref_image_face = detector.detect(ref_image);
    if (ref_image_face.size() != 1) {
        std::cerr << "Should have one face: " << ref_image_path.string() << std::endl;
        return 1;
    }
    cv::Mat ref_image_embedding = recognizer.embed(ref_image, ref_image_face.at(0));

    for (const auto &image_path: image_paths) {
        cv::Mat image = load_image(image_path, 1280);

        if (image.empty()) {
            skipped++;
            std::cout << "Skipped: " << image_path.string() << std::endl;
            continue;
        }

        double best_similarity_score = -1.1;
        double least_similarity_score = 1.1;
        std::vector<Face> faces = detector.detect(image);
        // if (faces.empty()) {continue;}
        for (const auto &face: faces) {
            cv::Mat face_embedding = recognizer.embed(image, face);
            double face_similarity_score = recognizer.similarity(face_embedding, ref_image_embedding);
            if (face_similarity_score > best_similarity_score) { best_similarity_score = face_similarity_score; }
            if (face_similarity_score < least_similarity_score) { least_similarity_score = face_similarity_score; }
        }

        loaded++;
        std::cout << "Loaded: " << image_path.string() << " - "
                << image.cols << " x " << image.rows << " - "
                << faces.size() << " faces detected" << " - "
                << "best (" << best_similarity_score << ") | least (" << least_similarity_score << ")"
                << std::endl;
    }
    std::cout << std::endl
            << "Loaded: " << loaded << " Images" << std::endl
            << "Skipped: " << skipped << " Images" << std::endl;

    return 0;
}
