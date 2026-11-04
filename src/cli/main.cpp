#include <algorithm>
#include <iostream>
#include <opencv2/core/utils/logger.hpp>

#include "album_directory.h"
#include "model_paths.h"
#include "output_directory.h"
#include "photo_finder.h"
#include "photo_results.h"
#include "photo_tagger.h"
#include "reference_directory.h"

// Images are shrunk to this size before detection.
static constexpr int kMaxDimension = 1280;

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

    // Model Directory: next to inframe.exe, so it works from any working directory.
    std::filesystem::path model_path(model_directory());
    if (!directory_exists(model_path)) return 1;

    // Face Detection Model
    std::filesystem::path face_detection_model_path = model_path / kDetectorModelFile;
    if (!model_exists(face_detection_model_path)) return 1;

    // Face Recognition Model
    std::filesystem::path face_recognition_model_path = model_path / kRecognizerModelFile;
    if (!model_exists(face_recognition_model_path)) return 1;

    PhotoFinder photo_finder;

    // Loading Reference Images
    std::cout << "Loading reference images" << std::endl;
    std::vector<std::filesystem::path> skipped_paths = photo_finder.load_references(reference_dir);
    for (const std::filesystem::path& image_path : skipped_paths) {
        std::cerr << "SKIPPED reference (needs exactly one face): " << image_path.string()
                  << std::endl;
    }

    int reference_size = photo_finder.reference_count();
    if (reference_size == 0) {
        std::cerr << "No usable reference images in " << reference_dir.directory_path().string()
                  << std::endl;
        return 1;
    }
    std::cout << "Using " << reference_size << " reference images" << std::endl;

    // Loading Album Directory
    std::cout << std::endl << "Searching album" << std::endl;

    int matched_count = 0;
    int skipped_count = 0;
    std::vector<PhotoResult> photo_results = photo_finder.process_album(album_dir);
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) {
            std::cerr << "SKIPPED (unreadable): " << photo_result.path.string() << std::endl;
            skipped_count++;
        } else if (photo_result.matched) {
            std::cout << "MATCHED: " << photo_result.path.string() << " ("
                      << photo_result.face_count << " faces, score " << photo_result.best_score
                      << ", " << photo_tag_name(photo_result.tag) << ")" << std::endl;
            matched_count++;
        }
    }

    // Summary
    int copy_count = PhotoFinder::copy_matches(photo_results, output_directory);
    std::cout << std::endl << "Summary" << std::endl;
    std::cout << "Matched " << matched_count << " of " << photo_results.size() << " photos ("
              << skipped_count << " unreadable)" << std::endl;
    std::cout << "Copied " << copy_count << " of " << matched_count << " to "
              << output_directory.directory_path().string() << std::endl;

    return 0;
}
