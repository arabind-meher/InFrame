#pragma once

#include <filesystem>
#include <vector>

#include "album_directory.h"
#include "face_detector.h"
#include "face_recognizer.h"
#include "output_directory.h"
#include "photo_results.h"
#include "reference_directory.h"

// Finds a person in an album: the one class the CLI and GUI call.
class PhotoFinder {
public:
    // Loads both models from the models directory.
    PhotoFinder();

    // Learns the person from photos with exactly one face; returns the skipped photos.
    std::vector<std::filesystem::path> load_references(const ReferenceDirectory& reference_dir);

    // Number of reference faces in use.
    int reference_count() const;

    // Detects, scores and tags one album photo.
    PhotoResult process_photo(const AlbumDirectory& album_dir, const std::filesystem::path& path);

    // process_photo for every photo in the album.
    std::vector<PhotoResult> process_album(const AlbumDirectory& album_dir);

    // Copies matched photos; returns how many were copied. Call output_dir.create() first.
    static int copy_matches(const std::vector<PhotoResult>& photo_results,
                            const OutputDirectory& output_dir);

private:
    FaceDetector detector_;
    FaceRecognizer recognizer_;

    // One embedding per usable reference photo.
    std::vector<cv::Mat> reference_embeddings_;

    // Highest similarity of any face to any reference (-1.0 if no faces).
    double best_similarity_score(const cv::Mat& image, const std::vector<Face>& faces);
};
