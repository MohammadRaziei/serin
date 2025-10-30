#include "toon.hpp"
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace toon {

// Constants
constexpr char LIST_ITEM_MARKER = '-';
constexpr char LIST_ITEM_PREFIX[] = "- ";
constexpr char COLON = ':';
constexpr char SPACE = ' ';
constexpr char OPEN_BRACKET = '[';
constexpr char CLOSE_BRACKET = ']';
constexpr char OPEN_BRACE = '{';
constexpr char CLOSE_BRACE = '}';
constexpr char BACKSLASH = '\\';
constexpr char DOUBLE_QUOTE = '"';
constexpr char NEWLINE = '\n';
constexpr char TAB = '\t';
constexpr char HASH = '#';

constexpr const char* NULL_LITERAL = "null";
constexpr const char* TRUE_LITERAL = "true";
constexpr const char* FALSE_LITERAL = "false";

// Utility functions
bool isPrimitive(const JsonValue& value) { return value.isPrimitive(); }
bool isObject(const JsonValue& value) { return value.isObject(); }
bool isArray(const JsonValue& value) { return value.isArray(); }

// Helper classes
class LineWriter {
private:
    std::ostringstream stream_;
    int indent_;
    
public:
    LineWriter(int indent) : indent_(indent) {}
    
    void push(int depth, const std::string& line) {
        stream_ << std::string(depth * indent_, SPACE) << line << NEWLINE;
    }
    
    void pushListItem(int depth, const std::string& line) {
        stream_ << std::string(depth * indent_, SPACE) << LIST_ITEM_PREFIX << line << NEWLINE;
    }
    
    std::string toString() const {
        std::string result = stream_.str();
        // Remove trailing newline
        if (!result.empty() && result.back() == NEWLINE) {
            result.pop_back();
        }
        return result;
    }
};

// Encoding helpers
std::string encodeKey(const std::string& key) {
    // Check if key needs quoting
    static std::regex identifier_pattern("^[a-zA-Z_][a-zA-Z0-9_.]*$");
    if (std::regex_match(key, identifier_pattern)) {
        return key;
    }
    // Quote the key
    std::string result;
    result += DOUBLE_QUOTE;
    for (char c : key) {
        if (c == DOUBLE_QUOTE || c == BACKSLASH) {
            result += BACKSLASH;
        }
        result += c;
    }
    result += DOUBLE_QUOTE;
    return result;
}

std::string encodePrimitive(const JsonPrimitive& primitive, Delimiter delimiter) {
    return std::visit([delimiter](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return NULL_LITERAL;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return value ? TRUE_LITERAL : FALSE_LITERAL;
        }
        else if constexpr (std::is_same_v<T, double>) {
            // Convert to string without scientific notation
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            // Check if string needs quoting
            bool needsQuoting = value.empty() || 
                               value.front() == SPACE || 
                               value.back() == SPACE ||
                               value == TRUE_LITERAL ||
                               value == FALSE_LITERAL ||
                               value == NULL_LITERAL;
            
            // Check for structural characters
            char activeDelimiter = static_cast<char>(delimiter);
            if (!needsQuoting) {
                needsQuoting = value.find(activeDelimiter) != std::string::npos ||
                              value.find(COLON) != std::string::npos ||
                              value.find(DOUBLE_QUOTE) != std::string::npos ||
                              value.find(BACKSLASH) != std::string::npos ||
                              value.find(LIST_ITEM_PREFIX) == 0;
            }
            
            if (!needsQuoting) {
                return value;
            }
            
            // Quote and escape the string
            std::string result;
            result += DOUBLE_QUOTE;
            for (char c : value) {
                if (c == DOUBLE_QUOTE || c == BACKSLASH) {
                    result += BACKSLASH;
                }
                result += c;
            }
            result += DOUBLE_QUOTE;
            return result;
        }
    }, primitive);
}

std::string encodeAndJoinPrimitives(const std::vector<JsonPrimitive>& primitives, Delimiter delimiter) {
    std::vector<std::string> encoded;
    for (const auto& primitive : primitives) {
        encoded.push_back(encodePrimitive(primitive, delimiter));
    }
    
    char delimChar = static_cast<char>(delimiter);
    std::ostringstream oss;
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (i > 0) oss << delimChar;
        oss << encoded[i];
    }
    return oss.str();
}

std::string formatHeader(size_t length, const std::string& key, 
                        const std::vector<std::string>& fields, 
                        Delimiter delimiter, bool lengthMarker) {
    std::ostringstream oss;
    
    if (!key.empty()) {
        oss << encodeKey(key);
    }
    
    oss << OPEN_BRACKET;
    if (lengthMarker) {
        oss << HASH;
    }
    oss << length;
    
    char delimChar = static_cast<char>(delimiter);
    if (delimiter != Delimiter::Comma) {
        oss << delimChar;
    }
    
    if (!fields.empty()) {
        oss << OPEN_BRACE;
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) oss << delimChar;
            oss << encodeKey(fields[i]);
        }
        oss << CLOSE_BRACE;
    }
    
    oss << CLOSE_BRACKET << COLON;
    return oss.str();
}

// Type checking helpers
bool isArrayOfPrimitives(const JsonArray& array) {
    return std::all_of(array.begin(), array.end(), [](const JsonValue& value) {
        return value.isPrimitive();
    });
}

bool isArrayOfObjects(const JsonArray& array) {
    return std::all_of(array.begin(), array.end(), [](const JsonValue& value) {
        return value.isObject();
    });
}

bool isArrayOfArrays(const JsonArray& array) {
    return std::all_of(array.begin(), array.end(), [](const JsonValue& value) {
        return value.isArray();
    });
}

// Main encoding functions
void encodeObject(const JsonObject& obj, LineWriter& writer, int depth, const EncodeOptions& options);
void encodeArray(const std::string& key, const JsonArray& array, LineWriter& writer, int depth, const EncodeOptions& options);

void encodeKeyValuePair(const std::string& key, const JsonValue& value, LineWriter& writer, int depth, const EncodeOptions& options) {
    if (value.isPrimitive()) {
        std::string line = encodeKey(key) + ": " + encodePrimitive(value.asPrimitive(), options.delimiter);
        writer.push(depth, line);
    }
    else if (value.isArray()) {
        encodeArray(key, value.asArray(), writer, depth, options);
    }
    else if (value.isObject()) {
        const JsonObject& obj = value.asObject();
        if (obj.empty()) {
            writer.push(depth, encodeKey(key) + ":");
        } else {
            writer.push(depth, encodeKey(key) + ":");
            encodeObject(obj, writer, depth + 1, options);
        }
    }
}

void encodeObject(const JsonObject& obj, LineWriter& writer, int depth, const EncodeOptions& options) {
    for (const auto& [key, value] : obj) {
        encodeKeyValuePair(key, value, writer, depth, options);
    }
}

void encodeArray(const std::string& key, const JsonArray& array, LineWriter& writer, int depth, const EncodeOptions& options) {
    if (array.empty()) {
        std::string header = formatHeader(0, key, {}, options.delimiter, options.lengthMarker);
        writer.push(depth, header);
        return;
    }
    
    // Primitive array
    if (isArrayOfPrimitives(array)) {
        std::vector<JsonPrimitive> primitives;
        for (const auto& value : array) {
            primitives.push_back(value.asPrimitive());
        }
        std::string joined = encodeAndJoinPrimitives(primitives, options.delimiter);
        std::string header = formatHeader(array.size(), key, {}, options.delimiter, options.lengthMarker);
        writer.push(depth, header + " " + joined);
        return;
    }
    
    // For now, handle mixed arrays as list items
    std::string header = formatHeader(array.size(), key, {}, options.delimiter, options.lengthMarker);
    writer.push(depth, header);
    
    for (const auto& item : array) {
        if (item.isPrimitive()) {
            writer.pushListItem(depth + 1, encodePrimitive(item.asPrimitive(), options.delimiter));
        } else if (item.isObject()) {
            // Simple object handling for now
            const JsonObject& obj = item.asObject();
            if (obj.empty()) {
                writer.pushListItem(depth + 1, "");
            } else {
                auto it = obj.begin();
                std::string firstLine = encodeKey(it->first) + ": " + encodePrimitive(it->second.asPrimitive(), options.delimiter);
                writer.pushListItem(depth + 1, firstLine);
                
                for (++it; it != obj.end(); ++it) {
                    encodeKeyValuePair(it->first, it->second, writer, depth + 2, options);
                }
            }
        }
    }
}

// Main encode function
std::string encode(const JsonValue& value, const EncodeOptions& options) {
    if (value.isPrimitive()) {
        return encodePrimitive(value.asPrimitive(), options.delimiter);
    }
    
    LineWriter writer(options.indent);
    
    if (value.isArray()) {
        encodeArray("", value.asArray(), writer, 0, options);
    }
    else if (value.isObject()) {
        encodeObject(value.asObject(), writer, 0, options);
    }
    
    return writer.toString();
}

// Decoding functions (simplified for now)
JsonValue decode(const std::string& input, const DecodeOptions& options) {
    // For now, return a simple implementation
    if (input.empty()) {
        return JsonObject{};
    }
    
    // Simple primitive detection
    if (input == "true") return JsonValue(true);
    if (input == "false") return JsonValue(false);
    if (input == "null") return JsonValue(nullptr);
    
    // Try to parse as number
    try {
        size_t pos;
        double num = std::stod(input, &pos);
        if (pos == input.length()) {
            return JsonValue(num);
        }
    } catch (...) {
        // Not a number
    }
    
    // Default to string
    return JsonValue(input);
}

// File I/O functions implementation
void encodeToFile(const JsonValue& value, const std::string& outputFile, const EncodeOptions& options) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + outputFile);
    }
    
    std::string toonContent = encode(value, options);
    file << toonContent;
}

JsonValue decodeFromFile(const std::string& inputFile, const DecodeOptions& options) {
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file: " + inputFile);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string toonContent = buffer.str();
    
    return decode(toonContent, options);
}

// JSON parsing functions implementation
JsonValue parseJson(const std::string& jsonString) {
    // Simple JSON parser implementation
    // For now, this is a placeholder that handles basic cases
    // In a real implementation, you would use a proper JSON parser
    
    std::string trimmed = jsonString;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    
    if (trimmed.empty()) {
        return JsonObject{};
    }
    
    // Check for object
    if (trimmed.front() == '{' && trimmed.back() == '}') {
        JsonObject obj;
        // Simple object parsing - in real implementation, parse properly
        return JsonValue(obj);
    }
    
    // Check for array
    if (trimmed.front() == '[' && trimmed.back() == ']') {
        JsonArray arr;
        // Simple array parsing - in real implementation, parse properly
        return JsonValue(arr);
    }
    
    // Check for string
    if (trimmed.front() == '"' && trimmed.back() == '"') {
        return JsonValue(trimmed.substr(1, trimmed.length() - 2));
    }
    
    // Check for number
    try {
        size_t pos;
        double num = std::stod(trimmed, &pos);
        if (pos == trimmed.length()) {
            return JsonValue(num);
        }
    } catch (...) {
        // Not a number
    }
    
    // Check for boolean
    if (trimmed == "true") return JsonValue(true);
    if (trimmed == "false") return JsonValue(false);
    if (trimmed == "null") return JsonValue(nullptr);
    
    // Default to string
    return JsonValue(trimmed);
}

JsonValue parseJsonFromFile(const std::string& inputFile) {
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file: " + inputFile);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseJson(buffer.str());
}

std::string toJsonString(const JsonValue& value, int indent) {
    // Simple JSON string conversion
    // In a real implementation, this would properly format JSON
    
    if (value.isPrimitive()) {
        const JsonPrimitive& primitive = value.asPrimitive();
        return std::visit([](const auto& val) -> std::string {
            using T = std::decay_t<decltype(val)>;
            
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return "null";
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return val ? "true" : "false";
            }
            else if constexpr (std::is_same_v<T, double>) {
                return std::to_string(val);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                return "\"" + val + "\"";
            }
        }, primitive);
    }
    else if (value.isObject()) {
        const JsonObject& obj = value.asObject();
        if (obj.empty()) {
            return "{}";
        }
        return "{ /* object with " + std::to_string(obj.size()) + " properties */ }";
    }
    else if (value.isArray()) {
        const JsonArray& arr = value.asArray();
        if (arr.empty()) {
            return "[]";
        }
        return "[ /* array with " + std::to_string(arr.size()) + " elements */ ]";
    }
    
    return "null";
}

// Update file I/O functions to use JSON parsing
std::string encodeFromFile(const std::string& inputFile, const EncodeOptions& options) {
    JsonValue jsonData = parseJsonFromFile(inputFile);
    return encode(jsonData, options);
}

void decodeToFile(const std::string& input, const std::string& outputFile, const DecodeOptions& options) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + outputFile);
    }
    
    JsonValue decoded = decode(input, options);
    std::string jsonOutput = toJsonString(decoded);
    file << jsonOutput;
}

// Update convertFile to use proper JSON parsing
void convertFile(const std::string& inputFile, const std::string& outputFile, 
                const EncodeOptions& encodeOptions, const DecodeOptions& decodeOptions) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(inputFile)) {
        throw std::runtime_error("Input file does not exist: " + inputFile);
    }
    
    std::string extension = fs::path(inputFile).extension().string();
    
    if (extension == ".json") {
        // JSON to TOON conversion
        JsonValue jsonData = parseJsonFromFile(inputFile);
        std::string toonContent = encode(jsonData, encodeOptions);
        
        if (outputFile.empty()) {
            std::cout << toonContent << std::endl;
        } else {
            std::ofstream file(outputFile);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot open output file: " + outputFile);
            }
            file << toonContent;
        }
    } else if (extension == ".toon") {
        // TOON to JSON conversion
        JsonValue decoded = decodeFromFile(inputFile, decodeOptions);
        std::string jsonContent = toJsonString(decoded);
        
        if (outputFile.empty()) {
            std::cout << jsonContent << std::endl;
        } else {
            std::ofstream file(outputFile);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot open output file: " + outputFile);
            }
            file << jsonContent;
        }
    } else {
        throw std::runtime_error("Unsupported file format: " + extension);
    }
}

} // namespace toon
