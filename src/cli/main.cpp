#include <CLI/CLI.hpp>
#include <exception>
#include <opencv2/core.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <string>

#include "commands.h"
#include "config.h"
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

    // Parse: help exits 0, any usage mistake exits 2.
    try {
        cli.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        int code = cli.exit(e);
        return code == 0 ? 0 : 2;
    }

    // Run: the one place runtime errors are caught.
    try {
        // Config: everything except album and refs comes from config.json.
        Config config = load_config(config_path());

        // Unknown keys are often typos; warn and carry on.
        for (const std::string& key : config.unknown_keys) {
            print_warning("Unknown config key \"" + key + "\" (ignored)");
        }

        // Settings from the config.
        find_options.output = config.output_directory;
        find_options.max_dimension = config.max_dimension;
        // model_directory is already absolute, so the joined paths are too.
        find_options.models.detector_path = config.model_directory / config.detector_model;
        find_options.models.recognizer_path = config.model_directory / config.recognizer_model;
        find_options.models.match_threshold = config.match_threshold;

        if (find_command->parsed()) return run_find(find_options);
    } catch (const cv::Exception& e) {
        // Most likely a corrupt model file, but any OpenCV failure ends up here.
        print_error(std::string("OpenCV failed (a model file may be corrupt):\n") + e.what());
        return 1;
    } catch (const std::exception& e) {
        // Also config errors: missing file, bad JSON, missing key, wrong value.
        print_error(e.what());
        return 1;
    }

    // Never reached: a subcommand always runs.
    return 0;
}
