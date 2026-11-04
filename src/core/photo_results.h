#pragma once

#include <filesystem>

#include "photo_tagger.h"

// What processing one album photo found.
struct PhotoResult {
    std::filesystem::path path;
    // False if the file couldn't be read.
    bool loaded = false;
    int face_count = 0;
    // Highest similarity to any reference; -1.0 if no faces.
    double best_score = -1.0;
    PhotoTag tag = PhotoTag::Scenery;
    // True if best_score reaches kMatchThreshold.
    bool matched = false;
};
