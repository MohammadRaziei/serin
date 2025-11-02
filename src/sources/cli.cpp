#include "serin.h"
#include "CLI11.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace fs = std::filesystem;

namespace {
constexpr const char *serinVersion = MACRO_STRINGIFY(SERIN_VERSION);

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::optional<serin::FormatType> parseFormat(const std::string &name) {
    const auto lowered = toLower(name);
    if (lowered == "json") {
        return serin::FormatType::JSON;
    }
    if (lowered == "toon") {
        return serin::FormatType::TOON;
    }
    if (lowered == "yaml" || lowered == "yml") {
        return serin::FormatType::YAML;
    }
    return std::nullopt;
}

std::string availableFormats() {
    return "json, toon, yaml";
}

void printHelp(const CLI::App &app) {
    std::cout << app.help() << std::endl;
}

} // namespace

int main(int argc, char **argv) {
    CLI::App app{"Serin - A modern C++ serialization library and CLI tool\n"
                 "Version: " + std::string(serinVersion) + "\n"
                 "\n"
                 "Serin provides fast and flexible serialization between JSON, YAML, and Toon formats. "
                 "It can convert between different serialization formats and manipulate structured data.\n"
                 "\n"
                 "Examples:\n"
                 "  serin input.json -o output.yaml          # Convert JSON to YAML\n"
                 "  serin input.yaml -t json                 # Convert YAML to JSON (stdout)\n"
                 "  serin input.toon -o output.json -i 4     # Convert Toon to JSON with 4-space indent"};

    std::string inputPath;
    std::string outputPath;
    std::string outputType;
    int indent = 2;
    bool showVersion = false;

    app.set_help_flag("-h,--help", "Show this help message and exit");
    app.add_option("input", inputPath, "Path to the input document (required)");
    app.add_option("-o,--output", outputPath, "Path to the output document (if omitted, prints to stdout)");
    app.add_option("-t,--type", outputType, "Output format: " + availableFormats() + " (default: toon)");
    app.add_option("-i,--indent", indent, "Indent level for structured output (default: 2)");
    app.add_flag("--version", showVersion, "Show version information and exit");

    if (argc == 1) {
        printHelp(app);
        return 0;
    }

    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp &help) {
        printHelp(app);
        return 0;
    } catch (const CLI::ParseError &error) {
        std::cerr << error.what() << std::endl;
        std::cerr << "Use --help or -h for more information" << std::endl;
        return 1;
    }

    if (showVersion) {
        std::cout << "serin " << serinVersion << std::endl;
        return 0;
    }

    if (inputPath.empty()) {
        printHelp(app);
        return 0;
    }

    if (indent < 0) {
        std::cerr << "Indent level must be non-negative" << std::endl;
        return 1;
    }

    if (!fs::exists(inputPath)) {
        std::cerr << "Input file not found: " << inputPath << std::endl;
        return 1;
    }

    try {
        // Load the file using auto-detection
        serin::Value value = serin::load(inputPath);

        if (!outputPath.empty()) {
            // Dump to file using auto-detection
            serin::dump(value, outputPath);
        } else {
            // Determine output format for stdout
            serin::FormatType outputFormat = serin::FormatType::TOON; // default
            if (!outputType.empty()) {
                auto requested = parseFormat(outputType);
                if (!requested) {
                    std::cerr << "Unknown output type: " << outputType << std::endl;
                    std::cerr << "Supported formats: " << availableFormats() << std::endl;
                    return 1;
                }
                outputFormat = *requested;
            }
            // Dump to stdout with specified format and indent
            std::cout << serin::dumps(value, outputFormat, indent) << std::endl;
        }
    } catch (const std::exception &error) {
        std::cerr << "Failed to process: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
