#include "commands.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <string>
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

    // Output: created only when copying (and only after validation).
    if (options.copy && !output_dir.create()) {
        print_error("Could not create output folder: " + display_path(output_dir.directory_path()));
        return 1;
    }
    if (options.copy) print_detail("Output folder: " + display_path(output_dir.directory_path()));

    // Exceptions (e.g. a corrupt model) are caught in main.
    PhotoFinder photo_finder(options.models);

    // References
    print_info("Loading reference images");
    std::vector<std::filesystem::path> skipped_paths = photo_finder.load_references(reference_dir);
    print_skipped_references(skipped_paths);

    int reference_size = photo_finder.reference_count();
    if (reference_size == 0) {
        print_no_references(reference_dir.directory_path());
        return 1;
    }
    print_reference_count(reference_size);

    // Album
    print_info("\nSearching album");
    std::vector<PhotoResult> photo_results = photo_finder.process_album(album_dir);
    print_results(photo_results);

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

    // No references loaded, so every photo gets faces and a tag but never matches.
    PhotoFinder photo_finder(options.models);

    print_info("Scanning album");
    std::vector<PhotoResult> photo_results = photo_finder.process_album(album_dir);
    print_scan_results(photo_results);
    print_scan_summary(photo_results);

    return 0;
}
