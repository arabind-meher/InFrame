#include "photo_finder.h"

#include <algorithm>

#include "model_paths.h"

PhotoFinder::PhotoFinder()
    : detector_(model_directory() / kDetectorModelFile),
      recognizer_(model_directory() / kRecognizerModelFile) {}

std::vector<std::filesystem::path> PhotoFinder::load_references(
    const ReferenceDirectory& reference_dir) {
    // Start fresh, so calling this twice doesn't mix two sets.
    reference_embeddings_.clear();

    std::vector<std::filesystem::path> skipped_paths;

    std::vector<std::filesystem::path> image_paths = reference_dir.find_images();
    for (const std::filesystem::path& image_path : image_paths) {
        cv::Mat image = reference_dir.load_image(image_path);
        if (image.empty()) {
            skipped_paths.push_back(image_path);
            continue;
        }

        std::vector<Face> image_faces = detector_.detect(image);

        // Several faces: we can't tell which one is the person.
        int n_faces = static_cast<int>(image_faces.size());
        if (n_faces != 1) {
            skipped_paths.push_back(image_path);
            continue;
        }

        reference_embeddings_.push_back(recognizer_.embed(image, image_faces.at(0)));
    }

    return skipped_paths;
}

int PhotoFinder::reference_count() const { return static_cast<int>(reference_embeddings_.size()); }

double PhotoFinder::best_similarity_score(const cv::Mat& image, const std::vector<Face>& faces) {
    double best_score = -1.0;

    for (const Face& face : faces) {
        cv::Mat face_embedding = recognizer_.embed(image, face);
        double score = recognizer_.best_similarity(face_embedding, reference_embeddings_);
        best_score = std::max(best_score, score);
    }

    return best_score;
}

PhotoResult PhotoFinder::process_photo(const AlbumDirectory& album_dir,
                                       const std::filesystem::path& path) {
    PhotoResult photo_result = {.path = path};

    // Unreadable: return with loaded = false.
    cv::Mat image = album_dir.load_image(path);
    if (image.empty()) {
        return photo_result;
    }

    photo_result.loaded = true;

    std::vector<Face> image_faces = detector_.detect(image);
    double best_score = best_similarity_score(image, image_faces);

    PhotoTag image_tag = classify_photo(image_faces);
    int face_count = static_cast<int>(image_faces.size());

    // A photo matches if any face is close enough to any reference.
    if (best_score >= kMatchThreshold) {
        photo_result.matched = true;
    }

    photo_result.face_count = face_count;
    photo_result.best_score = best_score;
    photo_result.tag = image_tag;

    return photo_result;
}

std::vector<PhotoResult> PhotoFinder::process_album(const AlbumDirectory& album_dir) {
    std::vector<PhotoResult> photo_results;

    std::vector<std::filesystem::path> image_paths = album_dir.find_images();
    for (const std::filesystem::path& image_path : image_paths) {
        photo_results.push_back(process_photo(album_dir, image_path));
    }

    return photo_results;
}

int PhotoFinder::copy_matches(const std::vector<PhotoResult>& photo_results,
                              const OutputDirectory& output_dir) {
    int count = 0;

    for (const PhotoResult& photo_result : photo_results) {
        if (photo_result.matched) {
            if (output_dir.copy(photo_result.path)) {
                count++;
            }
        }
    }

    return count;
}