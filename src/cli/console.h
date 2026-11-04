#pragma once

// All CLI printing lives here; the core never prints.

#include <filesystem>
#include <vector>

#include "photo_finder.h"
#include "photo_results.h"

// Prints one line per setup problem (to stderr).
void print_problem(const std::vector<SetupProblem>& setup_problems);

// Prints each reference photo that was skipped (to stderr).
void print_skipped_reference(const std::vector<std::filesystem::path>& skipped_paths);

// Prints a line for each unreadable or matched photo.
void print_results(const std::vector<PhotoResult>& photo_results);

// Prints match and copy counts at the end of a run.
void print_summary(const std::vector<PhotoResult>& photo_results, int copy_count,
                   const OutputDirectory& output_dir);
