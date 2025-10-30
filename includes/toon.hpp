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

// Utility functions
bool isPrimitive(const JsonValue& value);
bool isObject(const JsonValue& value);
bool isArray(const JsonValue& value);

} // namespace toon
