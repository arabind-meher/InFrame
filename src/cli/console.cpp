#include "console.h"

#include <iostream>
#include <string>

#include "model_paths.h"
#include "photo_tagger.h"

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

void print_problem(const std::vector<SetupProblem>& setup_problems) {
    // Errors go to stderr, so they still show when output is redirected.
    for (SetupProblem problem : setup_problems) {
        std::cerr << problem_message(problem) << std::endl;
    }
}

void print_skipped_reference(const std::vector<std::filesystem::path>& skipped_paths) {
    for (const std::filesystem::path& image_path : skipped_paths) {
        std::cerr << "SKIPPED reference (needs exactly one face): " << image_path.string()
                  << std::endl;
    }
}

void print_results(const std::vector<PhotoResult>& photo_results) {
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) {
            std::cerr << "SKIPPED (unreadable): " << photo_result.path.string() << std::endl;
        } else if (photo_result.matched) {
            // Non-matching photos print nothing.
            std::cout << "MATCHED: " << photo_result.path.string() << " ("
                      << photo_result.face_count << " faces, score " << photo_result.best_score
                      << ", " << photo_tag_name(photo_result.tag) << ")" << std::endl;
        }
    }
}

void print_summary(const std::vector<PhotoResult>& photo_results, int copy_count,
                   const OutputDirectory& output_dir) {
    // Count here, so main needs no counters.
    int matched_count = 0;
    int unreadable_count = 0;
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) unreadable_count++;
        if (photo_result.matched) matched_count++;
    }

    std::cout << std::endl << "Summary" << std::endl;
    std::cout << "Matched " << matched_count << " of " << photo_results.size() << " photos ("
              << unreadable_count << " unreadable)" << std::endl;
    std::cout << "Copied " << copy_count << " of " << matched_count << " to "
              << output_dir.directory_path().string() << std::endl;
}

void print_error(const std::string& message) {
    std::cerr << message << std::endl;
}
