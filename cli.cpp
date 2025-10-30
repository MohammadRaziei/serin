#include "includes/serin.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void printUsage() {
    std::cout << "Serin CLI - Serialization Library" << std::endl;
    std::cout << "Usage: toon <input-file> [-o <output-file>]" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  toon input.json              # Convert JSON to TOON (stdout)" << std::endl;
    std::cout << "  toon input.json -o output.toon  # Convert JSON to TOON file" << std::endl;
    std::cout << "  toon data.toon -o output.json   # Convert TOON to JSON file" << std::endl;
    std::cout << "  toon data.toon               # Convert TOON to JSON (stdout)" << std::endl;
    std::cout << std::endl;
    std::cout << "Supported formats: .json, .toon" << std::endl;
}

bool hasExtension(const std::string& filename, const std::string& extension) {
    if (filename.length() < extension.length()) return false;
    return filename.compare(filename.length() - extension.length(), extension.length(), extension) == 0;
}

std::string getOutputExtension(const std::string& inputFile) {
    if (hasExtension(inputFile, ".json")) {
        return ".toon";
    } else if (hasExtension(inputFile, ".toon")) {
        return ".json";
    }
    return "";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::vector<std::string> args(argv + 1, argv + argc);
    
    if (args.empty() || args[0] == "-h" || args[0] == "--help") {
        printUsage();
        return 0;
    }
    
    std::string inputFile;
    std::string outputFile;
    bool outputSpecified = false;
    
    // Simple argument parsing
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-o" && i + 1 < args.size()) {
            outputFile = args[i + 1];
            outputSpecified = true;
            i++; // Skip next argument
        } else if (inputFile.empty()) {
            inputFile = args[i];
        }
    }
    
    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        printUsage();
        return 1;
    }
    
    // Check if input file exists
    if (!fs::exists(inputFile)) {
        std::cerr << "Error: Input file does not exist: " << inputFile << std::endl;
        return 1;
    }
    
    // Auto-detect output file if not specified
    if (!outputSpecified) {
        std::string outputExt = getOutputExtension(inputFile);
        if (!outputExt.empty()) {
            std::string baseName = inputFile.substr(0, inputFile.find_last_of('.'));
            outputFile = baseName + outputExt;
        }
    }
    
    try {
        std::string extension = fs::path(inputFile).extension().string();
        
        if (extension == ".json") {
            // JSON to TOON conversion
            serin::Value jsonData = serin::parseJsonFromFile(inputFile);
            std::string toonContent = serin::encode(jsonData);
            
            if (outputFile.empty()) {
                // Output to stdout
                std::cout << toonContent << std::endl;
            } else {
                // Output to file
                std::ofstream file(outputFile);
                if (!file.is_open()) {
                    std::cerr << "Error: Cannot open output file: " << outputFile << std::endl;
                    return 1;
                }
                file << toonContent;
                std::cout << "Converted " << inputFile << " to " << outputFile << std::endl;
            }
        }
        else if (extension == ".toon") {
            // TOON to JSON conversion
            serin::Value decoded = serin::decodeFromFile(inputFile);
            std::string jsonContent = serin::toJsonString(decoded);
            
            if (outputFile.empty()) {
                // Output to stdout
                std::cout << jsonContent << std::endl;
            } else {
                // Output to file
                std::ofstream file(outputFile);
                if (!file.is_open()) {
                    std::cerr << "Error: Cannot open output file: " << outputFile << std::endl;
                    return 1;
                }
                file << jsonContent;
                std::cout << "Converted " << inputFile << " to " << outputFile << std::endl;
            }
        }
        else {
            std::cerr << "Error: Unsupported file format: " << extension << std::endl;
            std::cerr << "Supported formats: .json, .toon" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
