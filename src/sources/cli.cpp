#include "serin.h"
#include "CLI11.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

namespace {
constexpr const char *kSerinVersion = "0.1.0";

enum class Format {
    Json,
    Toon,
    Yaml,
};

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::optional<Format> parseFormat(const std::string &name) {
    const auto lowered = toLower(name);
    if (lowered == "json") {
        return Format::Json;
    }
    if (lowered == "toon") {
        return Format::Toon;
    }
    if (lowered == "yaml" || lowered == "yml") {
        return Format::Yaml;
    }
    return std::nullopt;
}

std::optional<Format> detectFormatFromExtension(const fs::path &path) {
    const auto ext = toLower(path.extension().string());
    if (ext == ".json") {
        return Format::Json;
    }
    if (ext == ".toon") {
        return Format::Toon;
    }
    if (ext == ".yaml" || ext == ".yml") {
        return Format::Yaml;
    }
    return std::nullopt;
}

std::string availableFormats() {
    return "json, toon, yaml";
}

serin::Value loadValue(const std::string &input, Format format) {
    switch (format) {
    case Format::Json:
        return serin::loadJson(input);
    case Format::Toon:
        return serin::loadToon(input);
    case Format::Yaml:
        return serin::loadYaml(input);
    }
    throw std::runtime_error("Unsupported input format");
}

void dumpToFile(const serin::Value &value, const std::string &output, Format format, int indent) {
    serin::ToonOptions toonOptions;
    toonOptions.setIndent(indent);
    switch (format) {
    case Format::Json:
        serin::dumpJson(value, output, indent);
        break;
    case Format::Toon:
        serin::dumpToon(value, output, toonOptions);
        break;
    case Format::Yaml:
        serin::dumpYaml(value, output, indent);
        break;
    }
}

std::string dumpToString(const serin::Value &value, Format format, int indent) {
    serin::ToonOptions toonOptions;
    toonOptions.setIndent(indent);
    switch (format) {
    case Format::Json:
        return serin::dumpsJson(value, indent);
    case Format::Toon:
        return serin::dumpsToon(value, toonOptions);
    case Format::Yaml:
        return serin::dumpsYaml(value, indent);
    }
    throw std::runtime_error("Unsupported output format");
}

void printHelp(const CLI::App &app) {
    std::cout << app.help() << std::endl;
}

} // namespace

int main(int argc, char **argv) {
    CLI::App app{"Serin - Serialization CLI"};

    std::string inputPath;
    std::string outputPath;
    std::string outputType;
    int indent = 2;
    bool showVersion = false;

    app.set_help_flag("-h,--help", "Show this help message");
    app.add_option("input", inputPath, "Path to the input document");
    app.add_option("-o,--output", outputPath, "Path to the output document");
    app.add_option("-t,--type", outputType, "Output format (" + availableFormats() + ")");
    app.add_option("-i,--indent", indent, "Indent level for structured output (default: 2)");
    app.add_flag("--version", showVersion, "Show version information");

    if (argc == 1) {
        printHelp(app);
        return 0;
    }

    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp &help) {
        std::cout << help.what() << std::endl;
        return 0;
    } catch (const CLI::ParseError &error) {
        std::cerr << error.what() << std::endl;
        std::cerr << std::endl;
        printHelp(app);
        return 1;
    }

    if (showVersion) {
        std::cout << "serin-cli " << kSerinVersion << std::endl;
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

    auto inputFormat = detectFormatFromExtension(inputPath);
    if (!inputFormat) {
        std::cerr << "Unable to determine input format for '" << inputPath << "'" << std::endl;
        std::cerr << "Supported formats: " << availableFormats() << std::endl;
        return 1;
    }

    std::optional<Format> outputFormat;
    if (!outputPath.empty()) {
        outputFormat = detectFormatFromExtension(outputPath);
        if (!outputFormat) {
            std::cerr << "Unable to determine output format for '" << outputPath << "'" << std::endl;
            std::cerr << "Supported formats: " << availableFormats() << std::endl;
            return 1;
        }
        if (!outputType.empty()) {
            auto requested = parseFormat(outputType);
            if (!requested) {
                std::cerr << "Unknown output type: " << outputType << std::endl;
                std::cerr << "Supported formats: " << availableFormats() << std::endl;
                return 1;
            }
            if (requested != outputFormat) {
                std::cerr << "Output type '" << outputType << "' does not match the extension of '" << outputPath
                          << "'" << std::endl;
                return 1;
            }
        }
    } else {
        const std::string typeName = outputType.empty() ? "toon" : outputType;
        outputFormat = parseFormat(typeName);
        if (!outputFormat) {
            std::cerr << "Unknown output type: " << typeName << std::endl;
            std::cerr << "Supported formats: " << availableFormats() << std::endl;
            return 1;
        }
    }

    try {
        const auto value = loadValue(inputPath, *inputFormat);
        if (!outputPath.empty()) {
            dumpToFile(value, outputPath, *outputFormat, indent);
        } else {
            std::cout << dumpToString(value, *outputFormat, indent) << std::endl;
        }
    } catch (const std::exception &error) {
        std::cerr << "Failed to process input: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
