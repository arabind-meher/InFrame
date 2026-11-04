#include "commands.h"

#include <filesystem>
#include <iostream>
#include <vector>

#include "album_directory.h"
#include "console.h"
#include "output_directory.h"
#include "photo_finder.h"
#include "photo_results.h"
#include "reference_directory.h"

// Images are shrunk to this size before detection.
static constexpr int kMaxDimension = 1280;

int run_find(const FindOptions& options) {
    // Album: photos to search. References: photos of the person to find.
    AlbumDirectory album_dir(options.album, kMaxDimension);
    ReferenceDirectory reference_dir(options.reference, kMaxDimension);
    OutputDirectory output_directory(options.output);

    // Check everything first and report all problems at once.
    std::vector<SetupProblem> problems =
        PhotoFinder::validate(album_dir, reference_dir, output_directory);
    print_problem(problems);
    if (!problems.empty()) return 1;

    // Output: created once here (only after validation), then matches are copied in.
    if (!output_directory.create()) {
        std::cerr << "Failed to create output folder: "
                  << output_directory.directory_path().string() << std::endl;
        return 1;
    }

    // Exceptions (e.g. a corrupt model) are caught in main.
    PhotoFinder photo_finder;

    // References
    std::cout << "Loading reference images" << std::endl;
    std::vector<std::filesystem::path> skipped_paths = photo_finder.load_references(reference_dir);
    print_skipped_reference(skipped_paths);

    int reference_size = photo_finder.reference_count();
    if (reference_size == 0) {
        std::cerr << "No usable reference images in " << reference_dir.directory_path().string()
                  << std::endl;
        return 1;
    }
    std::cout << "Using " << reference_size << " reference images" << std::endl;

    // Album
    std::cout << std::endl << "Searching album" << std::endl;
    std::vector<PhotoResult> photo_results = photo_finder.process_album(album_dir);
    print_results(photo_results);

    // Copy and summary
    int copy_count = PhotoFinder::copy_matches(photo_results, output_directory);
    print_summary(photo_results, copy_count, output_directory);

    return 0;
}