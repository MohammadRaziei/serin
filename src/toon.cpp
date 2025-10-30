#include "toon.hpp"
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>

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

} // namespace toon
