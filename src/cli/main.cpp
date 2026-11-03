#include <algorithm>
#include <iostream>
#include <opencv2/core/utils/logger.hpp>

#include "album_directory.h"
#include "face_detector.h"
#include "face_recognizer.h"
#include "output_directory.h"
#include "photo_tagger.h"
#include "reference_directory.h"

// Images are shrunk to this size before detection.
static constexpr int kMaxDimension = 1280;
// Model folder, relative to the working directory.
static std::string kModelPath("models");

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
static std::vector<cv::Mat> load_reference_embeddings(const ReferenceDirectory& reference_dir,
                                                      FaceDetector& detector,
                                                      FaceRecognizer& recognizer) {
    std::cout << "Loading reference images" << std::endl;

    std::vector<cv::Mat> reference_embeddings;

    std::vector<std::filesystem::path> image_paths = reference_dir.find_images();
    for (const std::filesystem::path& image_path : image_paths) {
        cv::Mat image = reference_dir.load_image(image_path);
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
    if (argc != 4) {
        std::cerr << "Usage: inframe <album_dir> <reference_dir> <output_dir>" << std::endl;
        return 2;
    }

    // Album: photos to search.
    AlbumDirectory album_dir(argv[1], kMaxDimension);
    if (!album_dir.exists()) {
        std::cerr << "Album directory does not exist: " << album_dir.directory_path().string()
                  << std::endl;
        return 1;
    }

    // References: photos of the person to find.
    ReferenceDirectory reference_dir(argv[2], kMaxDimension);
    if (!reference_dir.exists()) {
        std::cerr << "Reference directory does not exist: "
                  << reference_dir.directory_path().string() << std::endl;
        return 1;
    }

    // Output: created once here, then matches are copied in.
    OutputDirectory output_directory(argv[3]);
    if (!output_directory.create()) {
        std::cerr << "Failed to create output directory: "
                  << output_directory.directory_path().string() << std::endl;
        return 1;
    }

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
        std::cerr << "No usable reference images in " << reference_dir.directory_path().string()
                  << std::endl;
        return 1;
    }
    std::cout << "using " << reference_embeddings.size() << " reference images" << std::endl;

    // Loading Album Directory
    std::cout << "Loading Album Images" << std::endl;

    int matched = 0;
    int skipped = 0;
    std::vector<std::filesystem::path> matching_paths;
    std::vector<std::filesystem::path> image_paths = album_dir.find_images();

    for (const std::filesystem::path& image_path : image_paths) {
        cv::Mat image = album_dir.load_image(image_path);
        if (image.empty()) {
            std::cerr << "SKIPPED: " << image_path.string() << std::endl;
            skipped++;
            continue;
        }

        std::vector<Face> image_faces = detector.detect(image);
        double best_score =
            best_score_from_image(image, image_faces, recognizer, reference_embeddings);

        PhotoTag image_tag = classify_photo(image_faces);

        // A photo matches if any face is close enough to any reference.
        if (best_score >= kMatchThreshold) {
            matching_paths.push_back(image_path);
            std::cout << "MATCHED: (" << image_faces.size() << " faces)[" << best_score << "] ["
                      << photo_tag_name(image_tag) << "] " << image_path.string() << std::endl;
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
        if (output_directory.copy(path)) {
            std::cout << "Copied " << path.string() << std::endl;
            copy_count++;
        } else {
            std::cerr << "Failed to copy " << path.string() << std::endl;
        }
    }
    std::cout << "Copied " << copy_count << " of " << matching_paths.size() << std::endl;

    return 0;
}
