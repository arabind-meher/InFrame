#pragma once

#include <string>
#include <vector>

#include "face_detector.h"

// Photo type based on how many faces it has.
enum class PhotoTag { Scenery, Solo, Group };

// Classifies a photo by its number of faces.
PhotoTag classify_photo(const std::vector<Face>& faces);

// Tag name for printing, e.g. "group".
std::string photo_tag_name(PhotoTag tag);