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

// Prints one warning per skipped reference photo.
void print_skipped_references(const std::vector<std::filesystem::path>& skipped_paths);

// Prints how many reference photos are in use.
void print_reference_count(int count);

// Prints an error when no reference photo was usable.
void print_no_references(const std::filesystem::path& reference_dir);

// Warns per unreadable photo; a line per match (not quiet), per non-match (verbose).
void print_results(const std::vector<PhotoResult>& photo_results);

// Prints match counts at the end of a run.
void print_summary(const std::vector<PhotoResult>& photo_results);

// Prints how many matches were copied and where; warns if some failed.
void print_copy_result(const std::vector<PhotoResult>& photo_results, int copy_count,
                       const OutputDirectory& output_dir);

// Tells the user how to copy the matches (only if there are any).
void print_copy_hint(const std::vector<PhotoResult>& photo_results);
