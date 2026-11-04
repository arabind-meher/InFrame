#include <CLI/CLI.hpp>
#include <opencv2/core/utils/logger.hpp>

int main(int argc, char* argv[]) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    // App: exactly one subcommand must be given.
    CLI::App cli("Find a person in your photo album.");
    cli.name("inframe");
    cli.require_subcommand(1);

    // Parse: help exits 0, any usage mistake exits 2.
    try {
        cli.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        int code = cli.exit(e);
        return code == 0 ? 0 : 2;
    }

    // No subcommands yet; never reached.
    return 0;
}
