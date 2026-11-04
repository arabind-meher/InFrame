#include <CLI/CLI.hpp>
#include <exception>
#include <opencv2/core.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <string>

#include "commands.h"
#include "console.h"

int main(int argc, char* argv[]) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    // App: exactly one subcommand must be given.
    CLI::App cli("Find a person in your photo album.");
    cli.name("inframe");
    cli.require_subcommand(1);

    // find: CLI11 writes the flag values straight into find_options.
    FindOptions find_options;

    CLI::App* find_command = cli.add_subcommand("find", "Find photos of a person in an album");
    find_command->add_option("--album", find_options.album, "Folder of photos to search")
        ->required();
    find_command->add_option("--refs", find_options.reference, "Folder of photos of the person")
        ->required();
    // Optional: keeps "InFrame Output" when missing.
    find_command->add_option("--out", find_options.output, "Folder to copy matches into")
        ->capture_default_str();

    // Parse: help exits 0, any usage mistake exits 2.
    try {
        cli.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        int code = cli.exit(e);

        if (code == 0)
            return 0;
        else
            return 2;
    }

    // Run: the one place runtime errors are caught.
    try {
        if (find_command->parsed()) return run_find(find_options);
    } catch (const cv::Exception& e) {
        // Most likely a corrupt model file, but any OpenCV failure ends up here.
        print_error(std::string("OpenCV error (a model file may be corrupt):\n") + e.what());
        return 1;
    } catch (const std::exception& e) {
        print_error(std::string("Error: ") + e.what());
        return 1;
    }

    return 0;
}
