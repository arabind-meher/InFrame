#include "photo_tagger.h"

PhotoTag classify_photo(const std::vector<Face>& faces) {
    const int face_count = static_cast<int>(faces.size());

    if (face_count == 0) return PhotoTag::Scenery;

    if (face_count == 1) return PhotoTag::Solo;

    return PhotoTag::Group;
}

std::string photo_tag_name(PhotoTag tag) {
    switch (tag) {
        case PhotoTag::Scenery:
            return "scenery";
        case PhotoTag::Solo:
            return "solo";
        case PhotoTag::Group:
            return "group";
    }
    return "unknown";
}