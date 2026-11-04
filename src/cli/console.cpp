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

void print_info(const std::string& message) {
    std::cout << message << std::endl;
}

// Warnings and errors go to stderr, so they still show when output is redirected.
void print_warning(const std::string& message) {
    std::cerr << "Warning: " << message << std::endl;
}

void print_error(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
}

void print_problems(const std::vector<SetupProblem>& setup_problems) {
    for (SetupProblem problem : setup_problems) {
        print_error(problem_message(problem));
    }
}

void print_skipped_references(const std::vector<std::filesystem::path>& skipped_paths) {
    for (const std::filesystem::path& image_path : skipped_paths) {
        print_warning("Skipped reference (needs exactly one face): " + image_path.string());
    }
}

void print_reference_count(int count) {
    print_info("Using " + std::to_string(count) + " reference images");
}

void print_no_references(const std::filesystem::path& reference_dir) {
    print_error("No usable reference images in " + reference_dir.string());
}

void print_results(const std::vector<PhotoResult>& photo_results) {
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) {
            print_warning("Skipped (unreadable): " + photo_result.path.string());
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
    // Count here, so commands needs no counters.
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
