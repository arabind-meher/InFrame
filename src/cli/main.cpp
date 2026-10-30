#include <iostream>

#include "scanner.h"
#include "image_loader.h"

#include <opencv2/core/utils/logger.hpp>


int main(int argc, char *argv[]) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    if (argc != 2) {
        std::cerr << "Usage: inframe <folder>" << std::endl;
        return 2;
    }

    std::filesystem::path directory = argv[1];
    if (!std::filesystem::is_directory(directory)) {
        std::cerr << "Not a Directory: " << directory.string() << std::endl;
        return 1;
    }

    int loaded = 0;
    int skipped = 0;

    std::vector<std::filesystem::path> image_paths = find_images(directory);
    for (const auto &image_path: image_paths) {
        cv::Mat image = load_image(image_path, 1280);

        if (image.empty()) {
            skipped++;
            std::cout << "Skipped: " << image_path.string() << std::endl;
            continue;
        }

        loaded++;
        std::cout << "Loaded: " << image_path.string() << " - " << image.cols << " x " << image.rows << std::endl;
    }
    std::cout << std::endl
            << "Loaded: " << loaded << " Images" << std::endl
            << "Skipped: " << skipped << " Images" << std::endl;

    return 0;
}
