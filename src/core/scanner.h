#pragma once

#include <filesystem>
#include <vector>


// Returns all image files in directory and its subfolders
std::vector<std::filesystem::path> find_images(const std::filesystem::path &directory);
