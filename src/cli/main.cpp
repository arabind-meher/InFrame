#include <iostream>

#include "scanner.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: inframe <folder>" << std::endl;
        return 2;
    }

    std::filesystem::path directory = argv[1];
    if (!std::filesystem::is_directory(directory)) {
        std::cerr << "Not a Directory: " << directory.string() << std::endl;
        return 1;
    }

    std::vector<std::filesystem::path> images = find_images(directory);
    for (const auto &image: images) {
        std::cout << image.string() << std::endl;
    }
    std::cout << "Found " << images.size() << " images" << std::endl;

    return 0;
}
