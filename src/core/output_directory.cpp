#include "output_directory.h"

#include <string>
#include <system_error>
#include <utility>

OutputDirectory::OutputDirectory(std::filesystem::path directory)
    : directory_path_(std::move(directory)) {}

bool OutputDirectory::create() const {
    // Already existing is fine; only a real failure sets error.
    std::error_code error;
    std::filesystem::create_directories(directory_path_, error);

    return !error;
}

bool OutputDirectory::copy(const std::filesystem::path& image_path) const {
    std::filesystem::path target = directory_path_ / image_path.filename();

    // Same size means it was copied on an earlier run; otherwise try name_1, name_2, ...
    int counter = 1;
    while (std::filesystem::exists(target)) {
        if (std::filesystem::file_size(target) == std::filesystem::file_size(image_path)) {
            return true;
        }

        // Built as a path, so non-English names stay intact.
        std::filesystem::path new_name = image_path.stem();
        new_name += "_" + std::to_string(counter);
        new_name += image_path.extension();

        target = directory_path_ / new_name;
        counter++;
    }

    // error_code version: reports failure instead of throwing.
    std::error_code error;
    std::filesystem::copy_file(image_path, target, error);
    return !error;
}

std::filesystem::path OutputDirectory::directory_path() const { return directory_path_; }