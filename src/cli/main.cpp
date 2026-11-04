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

// Text for one setup problem.
static std::string problem_message(SetupProblem problem) {
    switch (problem) {
        case SetupProblem::AlbumMissing:
            return "Album folder not found.";
        case SetupProblem::ReferenceMissing:
            return "Reference folder not found.";
        case SetupProblem::OutputInsideAlbum:
            return "Output folder should not be inside the album folder.";
        case SetupProblem::DetectorModelMissing:
            return std::string("Model not found: ") + kDetectorModelFile;
        case SetupProblem::RecognizerModelMissing:
            return std::string("Model not found: ") + kRecognizerModelFile;
    }
    return "Unknown setup problem.";
}

int main(int argc, char* argv[]) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    // Check Arguments
    if (argc != 4) {
        std::cerr << "Usage: inframe <album_dir> <reference_dir> <output_dir>" << std::endl;
        return 2;
    }

    // Album: photos to search. References: photos of the person to find.
    AlbumDirectory album_dir(argv[1], kMaxDimension);
    ReferenceDirectory reference_dir(argv[2], kMaxDimension);
    OutputDirectory output_directory(argv[3]);

    // Check everything first and report all problems at once.
    std::vector<SetupProblem> problems =
        PhotoFinder::validate(album_dir, reference_dir, output_directory);
    for (SetupProblem problem : problems) {
        std::cerr << problem_message(problem) << std::endl;
    }
    if (!problems.empty()) return 1;

    // Output: created once here (only after validation), then matches are copied in.
    if (!output_directory.create()) {
        std::cerr << "Failed to create output folder: "
                  << output_directory.directory_path().string() << std::endl;
        return 1;
    }

    try {
        PhotoFinder photo_finder;

        // Loading Reference Images
        std::cout << "Loading reference images" << std::endl;
        std::vector<std::filesystem::path> skipped_paths =
            photo_finder.load_references(reference_dir);
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
    } catch (const cv::Exception& e) {
        // Most likely a corrupt model file, but any OpenCV failure ends up here.
        std::cerr << "OpenCV error (a model file may be corrupt):" << std::endl
                  << e.what() << std::endl;
        return 1;
    }

    return 0;
}
