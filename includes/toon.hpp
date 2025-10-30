#pragma once

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

namespace toon {

// Forward declarations
struct JsonValue;

// JSON value types
using JsonPrimitive = std::variant<std::string, double, bool, std::nullptr_t>;
using JsonObject = std::unordered_map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;

struct JsonValue {
    std::variant<JsonPrimitive, JsonObject, JsonArray> value;
    
    // Constructors
    JsonValue() : value(nullptr) {}
    JsonValue(const JsonPrimitive& p) : value(p) {}
    JsonValue(const JsonObject& o) : value(o) {}
    JsonValue(const JsonArray& a) : value(a) {}
    
    // Type checking
    bool isPrimitive() const { return std::holds_alternative<JsonPrimitive>(value); }
    bool isObject() const { return std::holds_alternative<JsonObject>(value); }
    bool isArray() const { return std::holds_alternative<JsonArray>(value); }
    
    // Getters
    const JsonPrimitive& asPrimitive() const { return std::get<JsonPrimitive>(value); }
    const JsonObject& asObject() const { return std::get<JsonObject>(value); }
    const JsonArray& asArray() const { return std::get<JsonArray>(value); }
    
    JsonPrimitive& asPrimitive() { return std::get<JsonPrimitive>(value); }
    JsonObject& asObject() { return std::get<JsonObject>(value); }
    JsonArray& asArray() { return std::get<JsonArray>(value); }
};

// Delimiter types
enum class Delimiter {
    Comma = ',',
    Tab = '\t',
    Pipe = '|'
};

// Encode options
struct EncodeOptions {
    int indent = 2;
    Delimiter delimiter = Delimiter::Comma;
    bool lengthMarker = false;
};

// Decode options
struct DecodeOptions {
    int indent = 2;
    bool strict = true;
};

// Main API functions
std::string encode(const JsonValue& value, const EncodeOptions& options = {});
JsonValue decode(const std::string& input, const DecodeOptions& options = {});

// JSON parsing functions
JsonValue parseJson(const std::string& jsonString);
JsonValue parseJsonFromFile(const std::string& inputFile);
std::string toJsonString(const JsonValue& value, int indent = 2);

// File I/O functions (equivalent to CLI functionality)
std::string encodeFromFile(const std::string& inputFile, const EncodeOptions& options = {});
void encodeToFile(const JsonValue& value, const std::string& outputFile, const EncodeOptions& options = {});
JsonValue decodeFromFile(const std::string& inputFile, const DecodeOptions& options = {});
void decodeToFile(const std::string& input, const std::string& outputFile, const DecodeOptions& options = {});

// Auto-detect and convert functions
void convertFile(const std::string& inputFile, const std::string& outputFile = "", const EncodeOptions& encodeOptions = {}, const DecodeOptions& decodeOptions = {});

// Utility functions
bool isPrimitive(const JsonValue& value);
bool isObject(const JsonValue& value);
bool isArray(const JsonValue& value);

} // namespace toon
