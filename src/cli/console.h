#pragma once

// All CLI printing lives here; the core never prints.

#include <filesystem>
#include <string>
#include <vector>

#include "output_directory.h"
#include "photo_finder.h"
#include "photo_results.h"

// Prints a normal message (to stdout).
void print_info(const std::string& message);

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

// Prints a warning per unreadable photo and a line per matched photo.
void print_results(const std::vector<PhotoResult>& photo_results);

// Prints match and copy counts at the end of a run.
void print_summary(const std::vector<PhotoResult>& photo_results, int copy_count,
                   const OutputDirectory& output_dir);
