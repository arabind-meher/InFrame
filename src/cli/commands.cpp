#include "commands.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "album_directory.h"
#include "console.h"
#include "output_directory.h"
#include "photo_finder.h"
#include "photo_results.h"
#include "reference_directory.h"

// Local start time as YYYYMMDD_HHMMSS (sortable, no ':' for Windows).
static std::string run_folder_name() {
    // Whole seconds, so %S prints no fractions.
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::floor<std::chrono::seconds>(now);

    // system_clock is UTC; zoned_time shows it in the Windows time zone.
    const std::chrono::zoned_time local{std::chrono::current_zone(), seconds};
    return std::format("{:%Y%m%d_%H%M%S}", local);
}

// process_album, but one photo at a time with a progress bar (label: "Searching", "Scanning").
static std::vector<PhotoResult> process_with_progress(
    PhotoFinder& photo_finder, const AlbumDirectory& album_dir,
    const std::vector<std::filesystem::path>& image_paths, std::string_view label) {
    std::vector<PhotoResult> photo_results;
    photo_results.reserve(image_paths.size());

    int total = static_cast<int>(image_paths.size());
    progress_start(label, total);
    for (int i = 0; i < total; i++) {
        photo_results.push_back(photo_finder.process_photo(album_dir, image_paths[i]));
        progress_update(i + 1);
    }
    progress_finish();

    return photo_results;
}

int run_find(const FindOptions& options) {
    // Album: photos to search. References: photos of the person to find.
    AlbumDirectory album_dir(options.album, options.max_dimension);
    ReferenceDirectory reference_dir(options.reference, options.max_dimension);
    // Output: a new date-time subfolder of the base folder for every run.
    OutputDirectory output_dir(options.output / run_folder_name());

    // Check everything first and report all problems at once.
    std::vector<SetupProblem> setup_problems =
        PhotoFinder::validate(album_dir, reference_dir, output_dir, options.models);
    print_problems(setup_problems);
    if (!setup_problems.empty()) return 1;

    // Header: album and its photo count; the list is reused for the loop below.
    std::vector<std::filesystem::path> image_paths = album_dir.find_images();
    print_album_line(options.album, static_cast<int>(image_paths.size()));

    // Output: created only when copying (and only after validation).
    if (options.copy && !output_dir.create()) {
        print_error("Could not create output folder: " + display_path(output_dir.directory_path()));
        return 1;
    }
    if (options.copy) print_detail("Output folder: " + display_path(output_dir.directory_path()));

    // Exceptions (e.g. a corrupt model) are caught in main.
    PhotoFinder photo_finder(options.models);

    // References: header line with used and skipped counts, then the skipped ones.
    std::vector<std::filesystem::path> skipped_paths = photo_finder.load_references(reference_dir);

    int reference_size = photo_finder.reference_count();
    if (reference_size == 0) {
        print_skipped_references(skipped_paths);
        print_no_references(reference_dir.directory_path());
        return 1;
    }
    print_reference_line(options.reference, reference_size, static_cast<int>(skipped_paths.size()));
    print_skipped_references(skipped_paths);

    // Album: one photo at a time, so the progress bar can follow.
    std::vector<PhotoResult> photo_results =
        process_with_progress(photo_finder, album_dir, image_paths, "Searching");
    print_results(photo_results, options.album);

    // Summary, then copy (with --copy) or a hint how to copy.
    print_summary(photo_results);
    if (options.copy) {
        int copy_count = PhotoFinder::copy_matches(photo_results, output_dir);
        print_copy_result(photo_results, copy_count, output_dir);
    } else {
        print_copy_hint(photo_results);
    }

    return 0;
}

int run_scan(const ScanOptions& options) {
    AlbumDirectory album_dir(options.album, options.max_dimension);

    // Only the album and models matter here.
    std::vector<SetupProblem> setup_problems = PhotoFinder::validate(album_dir, options.models);
    print_problems(setup_problems);
    if (!setup_problems.empty()) return 1;

    // Header: album and its photo count; the list is reused for the loop below.
    std::vector<std::filesystem::path> image_paths = album_dir.find_images();
    print_album_line(options.album, static_cast<int>(image_paths.size()));

    // No references loaded, so every photo gets faces and a tag but never matches.
    PhotoFinder photo_finder(options.models);

    std::vector<PhotoResult> photo_results =
        process_with_progress(photo_finder, album_dir, image_paths, "Scanning");
    print_scan_results(photo_results, options.album);
    print_scan_summary(photo_results);

    return 0;
}
