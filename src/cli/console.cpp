#include "console.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

#include "photo_tagger.h"
#include "style.h"

// Text for one setup problem.
static std::string problem_message(SetupProblem problem) {
    switch (problem) {
        case SetupProblem::AlbumMissing:
            return "Album folder not found.";
        case SetupProblem::ReferenceMissing:
            return "Reference folder not found.";
        case SetupProblem::OutputInsideAlbum:
            return "Output folder should not be inside the album folder.";
        case SetupProblem::DetectorModelMissing:
            return "Detector model not found (check model_directory and detector_model in "
                   "config.json).";
        case SetupProblem::RecognizerModelMissing:
            return "Recognizer model not found (check model_directory and recognizer_model in "
                   "config.json).";
    }
    return "Unknown setup problem.";
}

// The current output level, shared by every print function in this file.
static Verbosity current_verbosity = Verbosity::Normal;

void set_verbosity(Verbosity verbosity) { current_verbosity = verbosity; }

std::string display_path(const std::filesystem::path& path) {
    // make_preferred changes the path itself, so work on a copy.
    std::filesystem::path shown = path;
    shown.make_preferred();

    // UTF-8 bytes; string() would use the ANSI code page and throw on e.g. Hindi names.
    std::u8string utf8 = shown.u8string();
    return std::string(utf8.begin(), utf8.end());
}

// Characters, not bytes: UTF-8 continuation bytes (10xxxxxx) aren't counted.
static size_t text_width(const std::string& text) {
    size_t width = 0;
    for (unsigned char byte : text) {
        if ((byte & 0xC0) != 0x80) width++;
    }
    return width;
}

// "1 face", "2 faces": the count with the right word.
static std::string count_text(int count, std::string_view singular, std::string_view plural) {
    return std::format("{} {}", count, count == 1 ? singular : plural);
}

// Score with 2 decimals; "-" when there were no faces (score -1).
static std::string score_text(double score) {
    if (score < 0.0) return "-";
    return std::format("{:.2f}", score);
}

void print_info(const std::string& message) {
    if (current_verbosity == Verbosity::Quiet) return;
    std::cout << message << std::endl;
}

void print_detail(const std::string& message) {
    if (current_verbosity != Verbosity::Verbose) return;
    std::cout << styled(Style::Dim, "  " + message) << std::endl;
}

// Warnings and errors go to stderr, so they still show when output is redirected.
void print_warning(const std::string& message) {
    std::cerr << styled(Style::Warning, "Warning: " + message, Stream::Err) << std::endl;
}

void print_error(const std::string& message) {
    std::cerr << styled(Style::Error, "Error: " + message, Stream::Err) << std::endl;
}

void print_problems(const std::vector<SetupProblem>& setup_problems) {
    for (SetupProblem problem : setup_problems) {
        print_error(problem_message(problem));
    }
}

// "Label       value": labels in a 12-wide column, shared by the header and summaries.
static std::string label_line(std::string_view label, const std::string& value) {
    return std::format("  {:<12}{}", label, value);
}

void print_title() { print_info(styled(Style::Title, std::string("InFrame ") + INFRAME_VERSION)); }

void print_album_line(const std::filesystem::path& album_dir, int photo_count) {
    std::string note = count_text(photo_count, "photo", "photos");
    print_info(label_line("Album", std::format("{}  ({})", display_path(album_dir), note)));
}

void print_reference_line(const std::filesystem::path& reference_dir, int used_count,
                          int skipped_count) {
    std::string note = std::format("{} used, {} skipped", used_count, skipped_count);
    print_info(label_line("References", std::format("{}  ({})", display_path(reference_dir), note)));
}

// Progress state, like current_verbosity: one bar at a time.
static std::string progress_label;
static int progress_total = 0;
static bool progress_on = false;

// One frame: \r back to the line start, 20-cell bar, count; flushed since there's no newline.
static void draw_progress(int done) {
    constexpr int kBarWidth = 20;
    int filled = done * kBarWidth / progress_total;

    // "█" is 3 bytes in UTF-8, so append it in a loop (std::string(n, c) takes one char).
    std::string done_part;
    std::string todo_part;
    for (int i = 0; i < filled; i++) done_part += "█";
    for (int i = filled; i < kBarWidth; i++) todo_part += "░";

    std::cerr << std::format("\r{:<10} {}{}  {}/{}  ", progress_label,
                             styled(Style::Match, done_part, Stream::Err), todo_part, done,
                             progress_total)
              << std::flush;
}

void progress_start(std::string_view label, int total) {
    print_info("");

    progress_label = label;
    progress_total = total;
    progress_on = current_verbosity != Verbosity::Quiet && is_terminal(Stream::Err) && total > 0;

    if (progress_on) {
        draw_progress(0);
    } else {
        print_info(std::string(label) + " album");
    }
}

void progress_update(int done) {
    if (progress_on) draw_progress(done);
}

void progress_finish() {
    if (progress_on) std::cerr << std::endl;
    progress_on = false;
}

void print_skipped_references(const std::vector<std::filesystem::path>& skipped_paths) {
    for (const std::filesystem::path& image_path : skipped_paths) {
        print_warning("Skipped reference (needs exactly one face): " + display_path(image_path));
    }
}

void print_no_references(const std::filesystem::path& reference_dir) {
    print_error("No usable reference images in " + display_path(reference_dir));
}

// Path shown in tables: relative to the album, e.g. beach\IMG_0031.jpg.
static std::string table_path(const PhotoResult& photo_result,
                              const std::filesystem::path& album_dir) {
    return display_path(photo_result.path.lexically_relative(album_dir));
}

// Warnings come after a table, so they don't interleave with its rows.
static void print_unreadable(const std::vector<PhotoResult>& photo_results) {
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) {
            print_warning("Skipped (unreadable): " + display_path(photo_result.path));
        }
    }
}

// Which rows the find table shows at the current level.
static bool shows_row(const PhotoResult& photo_result) {
    if (!photo_result.loaded) return false;
    if (photo_result.matched) return current_verbosity != Verbosity::Quiet;
    return current_verbosity == Verbosity::Verbose;
}

void print_results(const std::vector<PhotoResult>& photo_results,
                   const std::filesystem::path& album_dir) {
    // Pass 1: width of the path column = longest path that will be shown.
    size_t path_width = 0;
    for (const PhotoResult& photo_result : photo_results) {
        if (shows_row(photo_result)) {
            path_width = std::max(path_width, text_width(table_path(photo_result, album_dir)));
        }
    }

    // Pass 2: one aligned row per shown photo; ✓ match, ✗ no match.
    // Pad first, colour after: colour codes would count as width and break the columns.
    if (path_width > 0) {
        std::cout << std::endl;
        for (const PhotoResult& photo_result : photo_results) {
            if (!shows_row(photo_result)) continue;
            std::string cells = std::format("{:<{}}  {:<8}  {:<8}  {:>4}",
                                            table_path(photo_result, album_dir), path_width,
                                            count_text(photo_result.face_count, "face", "faces"),
                                            photo_tag_name(photo_result.tag),
                                            score_text(photo_result.best_score));

            // Match: green status, readable row. No match: the whole row grey.
            if (photo_result.matched) {
                std::cout << "  " << styled(Style::Match, "✓") << "  " << cells << std::endl;
            } else {
                std::cout << styled(Style::Dim, "  ✗  " + cells) << std::endl;
            }
        }
    }

    print_unreadable(photo_results);
}

// Counted here, so commands needs no counters.
static int count_matched(const std::vector<PhotoResult>& photo_results) {
    int matched_count = 0;
    for (const PhotoResult& photo_result : photo_results) {
        if (photo_result.matched) matched_count++;
    }
    return matched_count;
}

static int count_unreadable(const std::vector<PhotoResult>& photo_results) {
    int unreadable_count = 0;
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) unreadable_count++;
    }
    return unreadable_count;
}

// Summary lines go straight to stdout, so they show even when quiet.
void print_summary(const std::vector<PhotoResult>& photo_results) {
    std::string matched =
        std::format("{} of {}", count_matched(photo_results),
                    count_text(static_cast<int>(photo_results.size()), "photo", "photos"));

    std::cout << std::endl << styled(Style::Heading, "Summary") << std::endl;
    std::cout << label_line("Matched", matched) << std::endl;
    std::cout << label_line("Unreadable", std::to_string(count_unreadable(photo_results)))
              << std::endl;
}

void print_copy_result(const std::vector<PhotoResult>& photo_results, int copy_count,
                       const OutputDirectory& output_dir) {
    int matched_count = count_matched(photo_results);

    std::string copied = std::format("{} of {} → {}", copy_count, matched_count,
                                     display_path(output_dir.directory_path()));
    std::cout << label_line("Copied", copied) << std::endl;

    // A failed copy doesn't stop the run, but the user should know.
    int failed_count = matched_count - copy_count;
    if (failed_count > 0) {
        print_warning(count_text(failed_count, "photo", "photos") + " could not be copied");
    }
}

void print_copy_hint(const std::vector<PhotoResult>& photo_results) {
    // No matches: nothing to copy, so no hint. Quiet: summary only.
    if (current_verbosity == Verbosity::Quiet) return;
    if (count_matched(photo_results) == 0) return;

    print_info(styled(Style::Dim, "Add --copy to copy them into a new output folder."));
}

void print_scan_results(const std::vector<PhotoResult>& photo_results,
                        const std::filesystem::path& album_dir) {
    // Quiet: summary only, but unreadable warnings still show.
    if (current_verbosity != Verbosity::Quiet) {
        // Pass 1: path column width.
        size_t path_width = 0;
        for (const PhotoResult& photo_result : photo_results) {
            if (photo_result.loaded) {
                path_width = std::max(path_width, text_width(table_path(photo_result, album_dir)));
            }
        }

        // Pass 2: no score column, scan has no references.
        if (path_width > 0) {
            std::cout << std::endl;
            for (const PhotoResult& photo_result : photo_results) {
                if (!photo_result.loaded) continue;
                std::cout << std::format("  {:<{}}  {:<8}  {}", table_path(photo_result, album_dir),
                                         path_width,
                                         count_text(photo_result.face_count, "face", "faces"),
                                         photo_tag_name(photo_result.tag))
                          << std::endl;
            }
        }
    }

    print_unreadable(photo_results);
}

void print_scan_summary(const std::vector<PhotoResult>& photo_results) {
    int scenery_count = 0;
    int solo_count = 0;
    int group_count = 0;

    // Unreadable photos have no real tag, so they are counted apart.
    for (const PhotoResult& photo_result : photo_results) {
        if (!photo_result.loaded) continue;
        switch (photo_result.tag) {
            case PhotoTag::Scenery:
                scenery_count++;
                break;
            case PhotoTag::Solo:
                solo_count++;
                break;
            case PhotoTag::Group:
                group_count++;
                break;
        }
    }

    std::cout << std::endl << styled(Style::Heading, "Summary") << std::endl;
    std::cout << label_line("Photos", std::to_string(photo_results.size())) << std::endl;
    std::cout << label_line("Scenery", std::to_string(scenery_count)) << std::endl;
    std::cout << label_line("Solo", std::to_string(solo_count)) << std::endl;
    std::cout << label_line("Group", std::to_string(group_count)) << std::endl;
    std::cout << label_line("Unreadable", std::to_string(count_unreadable(photo_results)))
              << std::endl;
}
