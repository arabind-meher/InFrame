#include "console.h"

#include <iostream>
#include <string>

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
            return "Detector model not found (check model_directory and detector_model in "
                   "config.json).";
        case SetupProblem::RecognizerModelMissing:
            return "Recognizer model not found (check model_directory and recognizer_model in "
                   "config.json).";
    }
    return "Unknown setup problem.";
}

// The current output level, shared by every print function in this file.
static Verbosity current_verbosity = Verbosity::Normal;

void set_verbosity(Verbosity verbosity) { current_verbosity = verbosity; }

void print_info(const std::string& message) {
    if (current_verbosity == Verbosity::Quiet) return;
    std::cout << message << std::endl;
}

void print_detail(const std::string& message) {
    if (current_verbosity != Verbosity::Verbose) return;
    std::cout << "  " << message << std::endl;
}

// Warnings and errors go to stderr, so they still show when output is redirected.
void print_warning(const std::string& message) { std::cerr << "Warning: " << message << std::endl; }

void print_error(const std::string& message) { std::cerr << "Error: " << message << std::endl; }

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

// One result line: label, path, faces, score and tag.
static void print_result_line(const std::string& label, const PhotoResult& photo_result) {
    std::cout << label << photo_result.path.string() << " (" << photo_result.face_count
              << " faces, score " << photo_result.best_score << ", "
              << photo_tag_name(photo_result.tag) << ")" << std::endl;
}

void print_results(const std::vector<PhotoResult>& photo_results) {
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) {
            // A warning, so it shows even when quiet.
            print_warning("Skipped (unreadable): " + photo_result.path.string());
        } else if (photo_result.matched) {
            if (current_verbosity != Verbosity::Quiet) print_result_line("MATCHED:  ", photo_result);
        } else {
            if (current_verbosity == Verbosity::Verbose) print_result_line("no match: ", photo_result);
        }
    }
}

// Counted here, so commands needs no counters.
static int count_matched(const std::vector<PhotoResult>& photo_results) {
    int matched_count = 0;
    for (const PhotoResult& photo_result : photo_results) {
        if (photo_result.matched) matched_count++;
    }
    return matched_count;
}

void print_summary(const std::vector<PhotoResult>& photo_results) {
    int unreadable_count = 0;
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) unreadable_count++;
    }

    std::cout << std::endl << "Summary" << std::endl;
    std::cout << "Matched " << count_matched(photo_results) << " of " << photo_results.size()
              << " photos (" << unreadable_count << " unreadable)" << std::endl;
}

void print_copy_result(const std::vector<PhotoResult>& photo_results, int copy_count,
                       const OutputDirectory& output_dir) {
    int matched_count = count_matched(photo_results);

    std::cout << "Copied " << copy_count << " of " << matched_count << " to "
              << output_dir.directory_path().string() << std::endl;

    // A failed copy doesn't stop the run, but the user should know.
    int failed_count = matched_count - copy_count;
    if (failed_count > 0) {
        print_warning(std::to_string(failed_count) + " photos could not be copied");
    }
}

void print_copy_hint(const std::vector<PhotoResult>& photo_results) {
    // No matches: nothing to copy, so no hint. Quiet: summary only.
    if (current_verbosity == Verbosity::Quiet) return;
    if (count_matched(photo_results) == 0) return;

    print_info("Add --copy to copy them into a new output folder.");
}

void print_scan_results(const std::vector<PhotoResult>& photo_results) {
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) {
            print_warning("Skipped (unreadable): " + photo_result.path.string());
        } else if (current_verbosity != Verbosity::Quiet) {
            // No score: scan has no references, so it is always -1.
            std::cout << photo_result.path.string() << " (" << photo_result.face_count
                      << " faces, " << photo_tag_name(photo_result.tag) << ")" << std::endl;
        }
    }
}

void print_scan_summary(const std::vector<PhotoResult>& photo_results) {
    int scenery_count = 0;
    int solo_count = 0;
    int group_count = 0;
    int unreadable_count = 0;

    // Unreadable photos have no real tag, so they are counted apart.
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) {
            unreadable_count++;
            continue;
        }
        switch (photo_result.tag) {
            case PhotoTag::Scenery:
                scenery_count++;
                break;
            case PhotoTag::Solo:
                solo_count++;
                break;
            case PhotoTag::Group:
                group_count++;
                break;
        }
    }

    std::cout << std::endl << "Summary" << std::endl;
    std::cout << "Scanned " << photo_results.size() << " photos: " << scenery_count << " scenery, "
              << solo_count << " solo, " << group_count << " group (" << unreadable_count
              << " unreadable)" << std::endl;
}
