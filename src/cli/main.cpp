#include <algorithm>
#include <iostream>
#include <opencv2/core/utils/logger.hpp>

#include "face_detector.h"
#include "face_recognizer.h"
#include "image_loader.h"
#include "image_scanner.h"

// Images are shrunk to this size before detection.
static constexpr int kMaxDimension = 1280;
// Model folder, relative to the working directory.
static std::string kModelPath("models");
// Testing only: matched photos are copied here for manual checking.
static std::filesystem::path destinationPath("tmp/matched");

// True if path is a folder; prints an error otherwise.
static bool directory_exists(const std::filesystem::path& path) {
    if (!std::filesystem::is_directory(path)) {
        std::cerr << "Directory does not exist: " << path << std::endl;
        return false;
    }
    return true;
}

// True if the model file exists; prints an error otherwise.
static bool model_exists(const std::filesystem::path& model_path) {
    if (!std::filesystem::exists(model_path)) {
        std::cerr << "Model does not exist: " << model_path << std::endl;
        return false;
    }
    return true;
}

// Embeds the face in each reference photo; skips photos without exactly one face.
static std::vector<cv::Mat> load_reference_embeddings(const std::filesystem::path& reference_dir,
                                                      FaceDetector& detector,
                                                      FaceRecognizer& recognizer) {
    std::cout << "Loading reference images" << std::endl;

    std::vector<cv::Mat> reference_embeddings;

    std::vector<std::filesystem::path> image_paths = find_images(reference_dir);
    for (const std::filesystem::path& image_path : image_paths) {
        cv::Mat image = load_image(image_path, kMaxDimension);
        std::vector<Face> image_faces = detector.detect(image);

        int n_faces = static_cast<int>(image_faces.size());
        if (n_faces != 1) {
            std::cerr << "SKIPPING IMAGE: (" << n_faces << " faces) - " << image_path.string()
                      << std::endl;
            continue;
        }

        std::cout << "LOADING IMAGE: (" << n_faces << " faces) - " << image_path.string()
                  << std::endl;
        reference_embeddings.push_back(recognizer.embed(image, image_faces.at(0)));
    }

    return reference_embeddings;
}

// Highest similarity of any face in the image to any reference (-1.0 if no faces).
static double best_score_from_image(const cv::Mat& image, const std::vector<Face>& faces,
                                    FaceRecognizer& recognizer,
                                    const std::vector<cv::Mat>& reference_embeddings) {
    double best_score = -1.0;

    for (const Face& face : faces) {
        cv::Mat face_embedding = recognizer.embed(image, face);
        double score = recognizer.best_similarity(face_embedding, reference_embeddings);
        best_score = std::max(best_score, score);
    }

    return best_score;
}

int main(int argc, char* argv[]) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    // Check Arguments
    if (argc != 3) {
        std::cerr << "Usage: inframe <album_dir> <reference_dir>" << std::endl;
        return 2;
    }

    // Check Argument Directories
    std::filesystem::path album_dir(argv[1]);
    if (!directory_exists(album_dir)) return 1;

    std::filesystem::path reference_dir(argv[2]);
    if (!directory_exists(reference_dir)) return 1;

    // Destination Directory (testing only)
    std::filesystem::path destination_dir(destinationPath);
    if (!directory_exists(destination_dir)) std::filesystem::create_directory(destination_dir);

    // Model Directory
    std::filesystem::path model_path(kModelPath);
    if (!directory_exists(model_path)) return 1;

    // Face Detection Model
    std::filesystem::path face_detection_model_path =
        model_path / "face_detection_yunet_2023mar.onnx";
    if (!model_exists(face_detection_model_path)) return 1;

    // Face Recognition Model
    std::filesystem::path face_recognition_model_path =
        model_path / "face_recognition_sface_2021dec.onnx";
    if (!model_exists(face_recognition_model_path)) return 1;

    // Loading Models
    FaceDetector detector(face_detection_model_path);
    FaceRecognizer recognizer(face_recognition_model_path);

    // Loading Reference Images
    std::vector<cv::Mat> reference_embeddings =
        load_reference_embeddings(reference_dir, detector, recognizer);
    if (reference_embeddings.empty()) {
        std::cerr << "No usable reference images in " << reference_dir.string() << std::endl;
        return 1;
    }
    std::cout << "using " << reference_embeddings.size() << " reference images" << std::endl;

    // Loading Album Directory
    std::cout << "Loading Album Images" << std::endl;

    int matched = 0;
    int skipped = 0;
    std::vector<std::filesystem::path> matching_paths;
    std::vector<std::filesystem::path> image_paths = find_images(album_dir);

    for (const std::filesystem::path& image_path : image_paths) {
        cv::Mat image = load_image(image_path, kMaxDimension);
        if (image.empty()) {
            std::cerr << "SKIPPED: " << image_path.string() << std::endl;
            skipped++;
            continue;
        }

        std::vector<Face> image_faces = detector.detect(image);
        double best_score =
            best_score_from_image(image, image_faces, recognizer, reference_embeddings);

        // A photo matches if any face is close enough to any reference.
        if (best_score >= kMatchThreshold) {
            matching_paths.push_back(image_path);
            std::cout << "MATCHED: (" << image_faces.size() << " faces)[" << best_score << "] "
                      << image_path.string() << std::endl;
            matched++;
        }
    }

    std::cout << std::endl << "Summary:" << std::endl;
    std::cout << "Photos you are in: " << std::endl;
    for (const std::filesystem::path& path : matching_paths) {
        std::cout << path.string() << std::endl;
    }
    std::cout << "Found you in " << matched << " of " << image_paths.size()
              << " photos and Skipped " << skipped << std::endl;

    // Testing only: copy matches so they can be checked by eye.
    std::cout << std::endl << "Copying matched images: " << std::endl;
    int copy_count = 0;
    for (const std::filesystem::path& path : matching_paths) {
        std::filesystem::path new_path = destination_dir / path.filename();
        std::filesystem::copy_file(path, new_path);
        copy_count++;
    }
    std::cout << "Copied " << copy_count << " of " << matching_paths.size() << std::endl;

    return 0;
}
