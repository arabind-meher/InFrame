#pragma once

// All CLI printing lives here; the core never prints.

#include <filesystem>
#include <string>
#include <vector>

#include "output_directory.h"
#include "photo_finder.h"
#include "photo_results.h"

// How much goes to stdout; errors and warnings (stderr) always print.
enum class Verbosity { Quiet, Normal, Verbose };

// Set once by main after parsing; Normal until then.
void set_verbosity(Verbosity verbosity);

// A path as shown to the user (Windows backslashes); used for every printed path.
std::string display_path(const std::filesystem::path& path);

// Prints a normal message (to stdout); hidden when quiet.
void print_info(const std::string& message);

// Prints an extra detail (to stdout); only when verbose.
void print_detail(const std::string& message);

// Prints "Warning: " and the message (to stderr).
void print_warning(const std::string& message);

// Prints "Error: " and the message (to stderr).
void print_error(const std::string& message);

// Prints one error per setup problem.
void print_problems(const std::vector<SetupProblem>& setup_problems);

// Header: program name and version.
void print_title();

// Header: album folder and how many photos it has.
void print_album_line(const std::filesystem::path& album_dir, int photo_count);

// Header: reference folder, how many photos were used and skipped.
void print_reference_line(const std::filesystem::path& reference_dir, int used_count,
                          int skipped_count);

// Prints one warning per skipped reference photo.
void print_skipped_references(const std::vector<std::filesystem::path>& skipped_paths);

// Prints an error when no reference photo was usable.
void print_no_references(const std::filesystem::path& reference_dir);

// Table of matches (not quiet) and non-matches (verbose), paths relative to the album;
// then a warning per unreadable photo.
void print_results(const std::vector<PhotoResult>& photo_results,
                   const std::filesystem::path& album_dir);

// Prints match counts at the end of a run.
void print_summary(const std::vector<PhotoResult>& photo_results);

// Prints how many matches were copied and where; warns if some failed.
void print_copy_result(const std::vector<PhotoResult>& photo_results, int copy_count,
                       const OutputDirectory& output_dir);

// Tells the user how to copy the matches (only if there are any).
void print_copy_hint(const std::vector<PhotoResult>& photo_results);

// scan: table of every photo with faces and tag (not quiet); then unreadable warnings.
void print_scan_results(const std::vector<PhotoResult>& photo_results,
                        const std::filesystem::path& album_dir);

// scan: photo count per tag at the end of a run.
void print_scan_summary(const std::vector<PhotoResult>& photo_results);
