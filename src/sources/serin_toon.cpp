#include "serin.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace serin {

ToonOptions::ToonOptions(int indent) {
    setIndent(indent);
}

ToonOptions& ToonOptions::setIndent(int indent) {
    indent_ = std::max(0, indent);
    return *this;
}

ToonOptions& ToonOptions::setDelimiter(Delimiter delimiter) {
    delimiter_ = delimiter;
    return *this;
}

ToonOptions& ToonOptions::setLengthMarker(bool enabled) {
    lengthMarker_ = enabled;
    return *this;
}

ToonOptions& ToonOptions::setStrict(bool strict) {
    strict_ = strict;
    return *this;
}

int ToonOptions::indent() const {
    return indent_;
}

Delimiter ToonOptions::delimiter() const {
    return delimiter_;
}

bool ToonOptions::lengthMarker() const {
    return lengthMarker_;
}

bool ToonOptions::strict() const {
    return strict_;
}

namespace {
struct EncodeOptions {
    int indent = 2;
    Delimiter delimiter = Delimiter::Comma;
    bool lengthMarker = false;
};

struct DecodeOptions {
    bool strict = true;
};

EncodeOptions makeEncodeOptions(const ToonOptions& options) {
    EncodeOptions result;
    result.indent = options.indent();
    result.delimiter = options.delimiter();
    result.lengthMarker = options.lengthMarker();
    return result;
}

DecodeOptions makeDecodeOptions(const ToonOptions& options) {
    DecodeOptions result;
    result.strict = options.strict();
    return result;
}

constexpr char COLON = ':';
constexpr char SPACE = ' ';
constexpr char OPEN_BRACKET = '[';
constexpr char CLOSE_BRACKET = ']';
constexpr char OPEN_BRACE = '{';
constexpr char CLOSE_BRACE = '}';
constexpr char DOUBLE_QUOTE = '"';
constexpr char BACKSLASH = '\\';
constexpr char NEWLINE = '\n';

constexpr const char* NULL_LITERAL = "null";
constexpr const char* TRUE_LITERAL = "true";
constexpr const char* FALSE_LITERAL = "false";

std::string encodePrimitive(const Primitive& primitive, Delimiter delimiter) {
    // Use Primitive::asString() which already uses yyjson for number serialization
    std::string result = primitive.asString();
    
    // For strings, we still need to handle TOON-specific quoting
    if (primitive.isString()) {
        bool needsQuoting = result.empty() || result.front() == SPACE || result.back() == SPACE ||
                             result == TRUE_LITERAL || result == FALSE_LITERAL || result == NULL_LITERAL;

        const char activeDelimiter = static_cast<char>(delimiter);
        if (!needsQuoting) {
            needsQuoting = result.find(activeDelimiter) != std::string::npos ||
                           result.find(COLON) != std::string::npos ||
                           result.find(DOUBLE_QUOTE) != std::string::npos ||
                           result.find(BACKSLASH) != std::string::npos;
        }

        if (!needsQuoting) {
            return result;
        }

        std::string escaped;
        escaped += DOUBLE_QUOTE;
        for (char c : result) {
            if (c == DOUBLE_QUOTE || c == BACKSLASH) {
                escaped += BACKSLASH;
            }
            escaped += c;
        }
        escaped += DOUBLE_QUOTE;
        return escaped;
    }
    
    return result;
}

std::string encodeAndJoinPrimitives(const std::vector<Primitive>& primitives, Delimiter delimiter) {
    std::vector<std::string> encoded;
    encoded.reserve(primitives.size());
    for (const auto& primitive : primitives) {
        encoded.push_back(encodePrimitive(primitive, delimiter));
    }

    std::ostringstream oss;
    const char delimChar = static_cast<char>(delimiter);
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (i > 0) {
            oss << delimChar;
        }
        oss << encoded[i];
    }
    return oss.str();
}

bool isArrayOfPrimitives(const Array& array) {
    return std::all_of(array.begin(), array.end(), [](const Value& value) {
        return value.isPrimitive();
    });
}

bool isArrayOfObjects(const Array& array) {
    return std::all_of(array.begin(), array.end(), [](const Value& value) {
        return value.isObject();
    });
}

std::string encodeArrayOfPrimitives(const std::string& key, const Array& array, const EncodeOptions& options) {
    std::ostringstream oss;
    oss << key << OPEN_BRACKET << array.size() << CLOSE_BRACKET << COLON;

    std::vector<Primitive> primitives;
    primitives.reserve(array.size());
    for (const auto& value : array) {
        primitives.push_back(value.asPrimitive());
    }

    oss << SPACE << encodeAndJoinPrimitives(primitives, options.delimiter);
    return oss.str();
}

std::string encodeArrayOfObjects(const std::string& key, const Array& array, const EncodeOptions& options, int depth);
std::string encodeObject(const Object& obj, const EncodeOptions& options, int depth);

std::string encodeValue(const std::string& key, const Value& value, const EncodeOptions& options, int depth) {
    if (value.isPrimitive()) {
        return key + COLON + SPACE + encodePrimitive(value.asPrimitive(), options.delimiter);
    }

    if (value.isArray()) {
        const Array& array = value.asArray();
        if (array.empty()) {
            return key + "[0]{}:";
        }

        if (isArrayOfPrimitives(array)) {
            return encodeArrayOfPrimitives(key, array, options);
        }

        if (isArrayOfObjects(array)) {
            return encodeArrayOfObjects(key, array, options, depth);
        }

        std::ostringstream oss;
        oss << key << OPEN_BRACKET << array.size() << CLOSE_BRACKET << COLON << NEWLINE;
        for (const auto& item : array) {
            oss << std::string((depth + 1) * options.indent, SPACE)
                << encodeValue("", item, options, depth + 1) << NEWLINE;
        }
        return oss.str();
    }

    if (value.isObject()) {
        const Object& obj = value.asObject();
        if (obj.empty()) {
            return key + COLON;
        }

        std::ostringstream oss;
        oss << key << COLON << NEWLINE;
        oss << encodeObject(obj, options, depth + 1);
        return oss.str();
    }

    return key + COLON + SPACE + NULL_LITERAL;
}

std::string encodeArrayOfObjects(const std::string& key, const Array& array, const EncodeOptions& options, int depth) {
    if (array.empty()) {
        return key + "[0]{}:\n";
    }

    const Object& firstObj = array.front().asObject();
    std::vector<std::string> fields;
    fields.reserve(firstObj.size());
    for (const auto& [field, _] : firstObj) {
        fields.push_back(field);
    }

    std::ostringstream oss;
    oss << key << OPEN_BRACKET << array.size() << CLOSE_BRACKET << OPEN_BRACE;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) {
            oss << static_cast<char>(options.delimiter);
        }
        oss << fields[i];
    }
    oss << CLOSE_BRACE << COLON << NEWLINE;

    bool first = true;
    for (const auto& item : array) {
        if (!item.isObject()) {
            continue;
        }

        const Object& obj = item.asObject();
        std::vector<Primitive> values;
        values.reserve(fields.size());
        for (const auto& field : fields) {
            const auto it = obj.find(field);
            if (it != obj.end() && it->second.isPrimitive()) {
                values.push_back(it->second.asPrimitive());
            } else {
                values.push_back(nullptr);
            }
        }
        if (!first) {
            oss << NEWLINE;
        }
        first = false;
        // Ensure all array items are indented with proper depth
        oss << std::string((depth + 1) * options.indent, SPACE) << encodeAndJoinPrimitives(values, options.delimiter);
    }

    return oss.str();
}

std::string encodeObject(const Object& obj, const EncodeOptions& options, int depth) {
    std::ostringstream oss;
    const std::string indent(depth * options.indent, SPACE);

    // The ordered_map preserves insertion order, so we can just iterate normally
    bool first = true;
    for (const auto& [key, value] : obj) {
        if (!first) {
            oss << NEWLINE;
        }
        oss << indent << encodeValue(key, value, options, depth);
        first = false;
    }

    return oss.str();
}

} // namespace

std::string encode(const Value& value, const EncodeOptions& options) {
    if (value.isPrimitive()) {
        return encodePrimitive(value.asPrimitive(), options.delimiter);
    }

    if (value.isArray()) {
        return encodeValue("", value, options, 0);
    }

    if (value.isObject()) {
        return encodeObject(value.asObject(), options, 0);
    }

    return NULL_LITERAL;
}

Value decode(const std::string& input, const DecodeOptions& /*options*/) {
    if (input.empty()) {
        return Object{};
    }

    if (input == TRUE_LITERAL) {
        return Value(true);
    }
    if (input == FALSE_LITERAL) {
        return Value(false);
    }
    if (input == NULL_LITERAL) {
        return Value(nullptr);
    }

    try {
        size_t pos = 0;
        const double num = std::stod(input, &pos);
        if (pos == input.length()) {
            return Value(num);
        }
    } catch (...) {
    }

    return Value(input);
}

void encodeToFile(const Value& value, const std::string& outputFile, const EncodeOptions& options) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + outputFile);
    }

    file << encode(value, options);
}

Value decodeFromFile(const std::string& inputFile, const DecodeOptions& options) {
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file: " + inputFile);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return decode(buffer.str(), options);
}

Value loadToon(const std::string& filename, const ToonOptions& options) {
    return decodeFromFile(filename, makeDecodeOptions(options));
}

Value loadsToon(const std::string& toonString, const ToonOptions& options) {
    return decode(toonString, makeDecodeOptions(options));
}

std::string dumpsToon(const Value& value, const ToonOptions& options) {
    return encode(value, makeEncodeOptions(options));
}

void dumpToon(const Value& value, const std::string& filename, const ToonOptions& options) {
    encodeToFile(value, filename, makeEncodeOptions(options));
}

} // namespace serin
