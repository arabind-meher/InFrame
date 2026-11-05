#include <CLI/CLI.hpp>
#include <exception>
#include <format>
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

    // --version prints and exits 0, like --help; the number comes from CMake's project VERSION.
    cli.set_version_flag("--version", "inframe " + std::string(INFRAME_VERSION));

    // Lets app flags like --quiet come after the subcommand too.
    cli.fallthrough();

    // Output level, for every subcommand; can't use both.
    bool quiet = false;
    bool verbose = false;
    CLI::Option* quiet_option = cli.add_flag("-q,--quiet", quiet, "Only print the summary");
    CLI::Option* verbose_option =
        cli.add_flag("-v,--verbose", verbose, "Also print non-matches and details");
    quiet_option->excludes(verbose_option);

    // find: CLI11 writes the flag values straight into find_options.
    FindOptions find_options;

    CLI::App* find_command = cli.add_subcommand("find", "Find photos of a person in an album");
    find_command->add_option("--album", find_options.album, "Folder of photos to search")
        ->required();
    find_command->add_option("--refs", find_options.reference, "Folder of photos of the person")
        ->required();
    find_command->add_flag("--copy", find_options.copy,
                           "Copy matched photos into a new output folder");

    // scan: album only; lists faces and tags, writes nothing.
    ScanOptions scan_options;

    CLI::App* scan_command = cli.add_subcommand("scan", "List face counts and tags for an album");
    scan_command->add_option("--album", scan_options.album, "Folder of photos to scan")->required();

    // Parse: help exits 0, any usage mistake exits 2.
    try {
        cli.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        int code = cli.exit(e);
        return code == 0 ? 0 : 2;
    }

    // Every print function checks this from now on.
    if (quiet)
        set_verbosity(Verbosity::Quiet);
    else if (verbose)
        set_verbosity(Verbosity::Verbose);
    else
        set_verbosity(Verbosity::Normal);

    // Run: the one place runtime errors are caught.
    try {
        // Config: everything except album and refs comes from config.json.
        Config config = load_config(config_path());

        // Unknown keys are often typos; warn and carry on.
        for (const std::string& key : config.unknown_keys) {
            print_warning("Unknown config key \"" + key + "\" (ignored)");
        }

        // Models: built once, shared by every subcommand.
        // model_directory is already absolute, so the joined paths are too.
        ModelSettings models;
        models.detector_path = config.model_directory / config.detector_model;
        models.recognizer_path = config.model_directory / config.recognizer_model;
        models.match_threshold = config.match_threshold;

        // Settings from the config; filled for both, only one subcommand runs.
        find_options.output = config.output_directory;
        find_options.max_dimension = config.max_dimension;
        find_options.models = models;
        scan_options.max_dimension = config.max_dimension;
        scan_options.models = models;

        // Verbose only: where the settings came from.
        print_detail("Config: " + display_path(config_path()));
        print_detail("Detector: " + display_path(models.detector_path));
        print_detail("Recognizer: " + display_path(models.recognizer_path));
        print_detail(std::format("Match threshold: {:.3f}", models.match_threshold));

        if (find_command->parsed()) return run_find(find_options);
        if (scan_command->parsed()) return run_scan(scan_options);
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
