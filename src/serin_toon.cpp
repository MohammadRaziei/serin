#include "serin.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace serin {

namespace {
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
    return std::visit([delimiter](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return NULL_LITERAL;
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? TRUE_LITERAL : FALSE_LITERAL;
        } else if constexpr (std::is_same_v<T, double>) {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        } else if constexpr (std::is_same_v<T, std::string>) {
            bool needsQuoting = value.empty() || value.front() == SPACE || value.back() == SPACE ||
                                 value == TRUE_LITERAL || value == FALSE_LITERAL || value == NULL_LITERAL;

            const char activeDelimiter = static_cast<char>(delimiter);
            if (!needsQuoting) {
                needsQuoting = value.find(activeDelimiter) != std::string::npos ||
                               value.find(COLON) != std::string::npos ||
                               value.find(DOUBLE_QUOTE) != std::string::npos ||
                               value.find(BACKSLASH) != std::string::npos;
            }

            if (!needsQuoting) {
                return value;
            }

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

std::string encodeArrayOfObjects(const std::string& key, const Array& array, const EncodeOptions& options);
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
            return encodeArrayOfObjects(key, array, options);
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

std::string encodeArrayOfObjects(const std::string& key, const Array& array, const EncodeOptions& options) {
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
        oss << encodeAndJoinPrimitives(values, options.delimiter) << NEWLINE;
    }

    return oss.str();
}

std::string encodeObject(const Object& obj, const EncodeOptions& options, int depth) {
    std::ostringstream oss;
    const std::string indent(depth * options.indent, SPACE);

    for (const auto& [key, value] : obj) {
        oss << indent << encodeValue(key, value, options, depth) << NEWLINE;
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

Value loadToon(const std::string& filename) {
    return decodeFromFile(filename);
}

Value loadsToon(const std::string& toonString) {
    return decode(toonString);
}

std::string dumpsToon(const Value& value, const EncodeOptions& options) {
    return encode(value, options);
}

void dumpToon(const Value& value, const std::string& filename, const EncodeOptions& options) {
    encodeToFile(value, filename, options);
}

} // namespace serin

