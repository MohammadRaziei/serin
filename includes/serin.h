#pragma once

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

namespace serin {

// Forward declarations
struct Value;

// TOON value types
using Primitive = std::variant<std::string, double, bool, std::nullptr_t>;
using Object = std::unordered_map<std::string, Value>;
using Array = std::vector<Value>;

struct Value {
    std::variant<Primitive, Object, Array> value;
    
    // Constructors
    Value() : value(nullptr) {}
    Value(const Primitive& p) : value(p) {}
    Value(const Object& o) : value(o) {}
    Value(const Array& a) : value(a) {}
    
    // Type checking
    bool isPrimitive() const { return std::holds_alternative<Primitive>(value); }
    bool isObject() const { return std::holds_alternative<Object>(value); }
    bool isArray() const { return std::holds_alternative<Array>(value); }
    
    // Getters
    const Primitive& asPrimitive() const { return std::get<Primitive>(value); }
    const Object& asObject() const { return std::get<Object>(value); }
    const Array& asArray() const { return std::get<Array>(value); }
    
    Primitive& asPrimitive() { return std::get<Primitive>(value); }
    Object& asObject() { return std::get<Object>(value); }
    Array& asArray() { return std::get<Array>(value); }
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

// Utility functions
bool isPrimitive(const Value& value);
bool isObject(const Value& value);
bool isArray(const Value& value);

// Serialization functions (load/loads/dumps/dump)
// JSON functions
Value loadJson(const std::string& filename);
Value loadsJson(const std::string& jsonString);
std::string dumpsJson(const Value& value, int indent = 2);
void dumpJson(const Value& value, const std::string& filename, int indent = 2);

// TOON functions
Value loadToon(const std::string& filename);
Value loadsToon(const std::string& toonString);
std::string dumpsToon(const Value& value, int indent = 2);
void dumpToon(const Value& value, const std::string& filename, int indent = 2);

// YAML functions
Value loadYaml(const std::string& filename);
Value loadsYaml(const std::string& yamlString);
std::string dumpsYaml(const Value& value, int indent = 2);
void dumpYaml(const Value& value, const std::string& filename, int indent = 2);

} // namespace serin
