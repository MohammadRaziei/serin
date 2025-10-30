#include "serin.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace serin {

bool isPrimitive(const Value& value) { return value.isPrimitive(); }
bool isObject(const Value& value) { return value.isObject(); }
bool isArray(const Value& value) { return value.isArray(); }

std::string encodeFromFile(const std::string& inputFile, const EncodeOptions& options) {
    Value data = parseJsonFromFile(inputFile);
    return encode(data, options);
}

void decodeToFile(const std::string& input, const std::string& outputFile, const DecodeOptions& options) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + outputFile);
    }

    Value decoded = decode(input, options);
    file << toJsonString(decoded);
}

} // namespace serin

