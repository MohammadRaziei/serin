#include "serin.h"
#include "yyjson.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace serin {

// Primitive type checking methods
bool Primitive::isString() const { return std::holds_alternative<std::string>(*this); }
bool Primitive::isDouble() const { return std::holds_alternative<double>(*this); }
bool Primitive::isInt() const { return std::holds_alternative<int64_t>(*this); }
bool Primitive::isBool() const { return std::holds_alternative<bool>(*this); }
bool Primitive::isNull() const { return std::holds_alternative<std::nullptr_t>(*this); }

// Primitive getter methods with error checking
const std::string& Primitive::getString() const { 
    if (!isString()) {
        throw std::runtime_error("Primitive is not a string");
    }
    return std::get<std::string>(*this);
}

double Primitive::getDouble() const { 
    if (!isDouble()) {
        throw std::runtime_error("Primitive is not a double");
    }
    return std::get<double>(*this);
}

int64_t Primitive::getInt() const { 
    if (!isInt()) {
        throw std::runtime_error("Primitive is not an int");
    }
    return std::get<int64_t>(*this);
}

bool Primitive::getBool() const { 
    if (!isBool()) {
        throw std::runtime_error("Primitive is not a bool");
    }
    return std::get<bool>(*this);
}

std::nullptr_t Primitive::getNull() const { 
    if (!isNull()) {
        throw std::runtime_error("Primitive is not null");
    }
    return std::get<std::nullptr_t>(*this);
}

// Number methods
bool Primitive::isNumber() const { 
    return isDouble() || isInt();
}

double Primitive::getNumber() const { 
    if (isDouble()) {
        return getDouble();
    } else if (isInt()) {
        return static_cast<double>(getInt());
    } else {
        throw std::runtime_error("Primitive is not a number");
    }
}

// Primitive conversion to string
std::string Primitive::asString() const {
    return std::visit([](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        } else if constexpr (std::is_same_v<T, double>) {
            // Use yyjson for double serialization
            char buffer[64];
            yyjson_val val;
            unsafe_yyjson_set_real(&val, value);
            char *result = yyjson_write_number(&val, buffer);
            if (result) {
                return std::string(buffer, result - buffer);
            }
            // Fallback to stringstream if yyjson fails
            std::ostringstream oss;
            oss << value;
            return oss.str();
        } else if constexpr (std::is_same_v<T, int64_t>) {
            // Use yyjson for int64_t serialization
            char buffer[32];
            yyjson_val val;
            unsafe_yyjson_set_sint(&val, value);
            char *result = yyjson_write_number(&val, buffer);
            if (result) {
                return std::string(buffer, result - buffer);
            }
            // Fallback to stringstream if yyjson fails
            std::ostringstream oss;
            oss << value;
            return oss.str();
        } else { // std::string
            return value;
        }
    }, *this);
}

bool isPrimitive(const Value& value) { return value.isPrimitive(); }
bool isObject(const Value& value) { return value.isObject(); }
bool isArray(const Value& value) { return value.isArray(); }

// Generic file format functions (auto-detect format from file extension)
Value load(const std::string& filename) {
    // Extract file extension
    std::filesystem::path filePath(filename);
    std::string extension = filePath.extension().string();
    
    // Convert to lowercase for case-insensitive comparison
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // Route to appropriate loader based on file extension
    if (extension == ".json") {
        return loadJson(filename);
    } else if (extension == ".toon") {
        return loadToon(filename);
    } else if (extension == ".yaml" || extension == ".yml") {
        return loadYaml(filename);
    } else {
        throw std::runtime_error("Unsupported file format: " + extension + 
                               ". Supported formats: .json, .toon, .yaml, .yml");
    }
}

void dump(const Value& value, const std::string& filename) {
    // Extract file extension
    std::filesystem::path filePath(filename);
    std::string extension = filePath.extension().string();
    
    // Convert to lowercase for case-insensitive comparison
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // Route to appropriate dumper based on file extension
    if (extension == ".json") {
        dumpJson(value, filename);
    } else if (extension == ".toon") {
        dumpToon(value, filename);
    } else if (extension == ".yaml" || extension == ".yml") {
        dumpYaml(value, filename);
    } else {
        throw std::runtime_error("Unsupported file format: " + extension + 
                               ". Supported formats: .json, .toon, .yaml, .yml");
    }
}

Value loads(const std::string& content, FormatType format) {
    switch (format) {
        case FormatType::JSON:
            return loadsJson(content);
        case FormatType::TOON:
            return loadsToon(content);
        case FormatType::YAML:
            return loadsYaml(content);
        default:
            throw std::runtime_error("Unsupported format type");
    }
}

std::string dumps(const Value& value, FormatType format, int indent) {
    switch (format) {
        case FormatType::JSON:
            return dumpsJson(value, indent);
        case FormatType::TOON:
            return dumpsToon(value, EncoderOptions(indent));
        case FormatType::YAML:
            return dumpsYaml(value, indent);
        default:
            throw std::runtime_error("Unsupported format type");
    }
}

} // namespace serin
