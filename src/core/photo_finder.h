#pragma once

#include <filesystem>
#include <vector>

#include "album_directory.h"
#include "face_detector.h"
#include "face_recognizer.h"
#include "output_directory.h"
#include "photo_results.h"
#include "reference_directory.h"

// Something wrong with the setup, found before any work starts.
enum class SetupProblem {
    AlbumMissing,
    ReferenceMissing,
    OutputInsideAlbum,
    DetectorModelMissing,
    RecognizerModelMissing
};

// Which models to load and what score counts as a match; filled by the caller.
struct ModelSettings {
    std::filesystem::path detector_path;
    std::filesystem::path recognizer_path;
    // Lowest similarity score that counts as a match (belongs to the recognizer model).
    double match_threshold;
};

// Finds a person in an album: the one class the CLI and GUI call.
class PhotoFinder {
public:
    // Loads both models; call validate first so missing files are reported nicely.
    explicit PhotoFinder(const ModelSettings& model_settings);

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

    // Checks directories and models before constructing. Empty means OK.
    static std::vector<SetupProblem> validate(const AlbumDirectory& album_dir,
                                              const ReferenceDirectory& reference_dir,
                                              const OutputDirectory& output_dir,
                                              const ModelSettings& model_settings);

private:
    FaceDetector detector_;
    FaceRecognizer recognizer_;
    // From ModelSettings; used by process_photo.
    double match_threshold_;

    // One embedding per usable reference photo.
    std::vector<cv::Mat> reference_embeddings_;

    // Highest similarity of any face to any reference (-1.0 if no faces).
    double best_similarity_score(const cv::Mat& image, const std::vector<Face>& faces);
};
