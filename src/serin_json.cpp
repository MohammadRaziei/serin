#include "serin.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace serin {

namespace {
constexpr const char* NULL_LITERAL = "null";
constexpr const char* TRUE_LITERAL = "true";
constexpr const char* FALSE_LITERAL = "false";

class JsonParser {
public:
    explicit JsonParser(const std::string& source) : input(source) {}

    Value parse() {
        skipWhitespace();
        Value result = parseValue();
        skipWhitespace();
        if (pos != input.size()) {
            throw std::runtime_error("Unexpected trailing characters in JSON input");
        }
        return result;
    }

private:
    const std::string& input;
    size_t pos = 0;

    void skipWhitespace() {
        while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos]))) {
            ++pos;
        }
    }

    char peek() const {
        if (pos >= input.size()) {
            throw std::runtime_error("Unexpected end of JSON input");
        }
        return input[pos];
    }

    bool match(char expected) {
        if (pos < input.size() && input[pos] == expected) {
            ++pos;
            return true;
        }
        return false;
    }

    bool startsWith(const char* literal) const {
        const size_t len = std::strlen(literal);
        if (pos + len > input.size()) {
            return false;
        }
        return input.compare(pos, len, literal) == 0;
    }

    Value parseValue() {
        skipWhitespace();
        if (pos >= input.size()) {
            throw std::runtime_error("Unexpected end of JSON input");
        }

        const char current = input[pos];
        if (current == '"') {
            return Value(Primitive(parseString()));
        }
        if (current == '{') {
            return Value(parseObject());
        }
        if (current == '[') {
            return Value(parseArray());
        }
        if (current == '-' || std::isdigit(static_cast<unsigned char>(current))) {
            return Value(parseNumber());
        }
        if (startsWith(TRUE_LITERAL)) {
            pos += std::strlen(TRUE_LITERAL);
            return Value(Primitive(true));
        }
        if (startsWith(FALSE_LITERAL)) {
            pos += std::strlen(FALSE_LITERAL);
            return Value(Primitive(false));
        }
        if (startsWith(NULL_LITERAL)) {
            pos += std::strlen(NULL_LITERAL);
            return Value(Primitive(nullptr));
        }

        throw std::runtime_error("Invalid JSON value");
    }

    std::string parseString() {
        if (!match('"')) {
            throw std::runtime_error("Expected opening quote at beginning of JSON string");
        }

        std::string result;
        while (pos < input.size()) {
            const char c = input[pos++];
            if (c == '"') {
                return result;
            }
            if (c == '\\') {
                if (pos >= input.size()) {
                    throw std::runtime_error("Invalid escape sequence in JSON string");
                }
                const char escaped = input[pos++];
                switch (escaped) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        if (pos + 4 > input.size()) {
                            throw std::runtime_error("Invalid unicode escape in JSON string");
                        }
                        unsigned int codepoint = 0;
                        for (int i = 0; i < 4; ++i) {
                            const char hex = input[pos++];
                            codepoint <<= 4;
                            if (hex >= '0' && hex <= '9') {
                                codepoint |= (hex - '0');
                            } else if (hex >= 'a' && hex <= 'f') {
                                codepoint |= (hex - 'a' + 10);
                            } else if (hex >= 'A' && hex <= 'F') {
                                codepoint |= (hex - 'A' + 10);
                            } else {
                                throw std::runtime_error("Invalid character in unicode escape");
                            }
                        }

                        if (codepoint <= 0x7F) {
                            result += static_cast<char>(codepoint);
                        } else if (codepoint <= 0x7FF) {
                            result += static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F));
                            result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid escape character in JSON string");
                }
            } else {
                result += c;
            }
        }

        throw std::runtime_error("Unterminated JSON string");
    }

    Primitive parseNumber() {
        const size_t start = pos;
        if (input[pos] == '-') {
            ++pos;
        }

        if (pos >= input.size()) {
            throw std::runtime_error("Invalid number in JSON");
        }

        if (input[pos] == '0') {
            ++pos;
        } else if (std::isdigit(static_cast<unsigned char>(input[pos]))) {
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                ++pos;
            }
        } else {
            throw std::runtime_error("Invalid number in JSON");
        }

        if (pos < input.size() && input[pos] == '.') {
            ++pos;
            if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos]))) {
                throw std::runtime_error("Invalid number in JSON");
            }
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                ++pos;
            }
        }

        if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
            ++pos;
            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
                ++pos;
            }
            if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos]))) {
                throw std::runtime_error("Invalid number in JSON");
            }
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                ++pos;
            }
        }

        const double value = std::stod(input.substr(start, pos - start));
        return Primitive(value);
    }

    Object parseObject() {
        if (!match('{')) {
            throw std::runtime_error("Expected '{' at beginning of JSON object");
        }
        skipWhitespace();

        Object object;
        if (match('}')) {
            return object;
        }

        while (true) {
            skipWhitespace();
            std::string key = parseString();
            skipWhitespace();
            if (!match(':')) {
                throw std::runtime_error("Expected ':' in JSON object");
            }
            skipWhitespace();
            Value value = parseValue();
            object.emplace(std::move(key), std::move(value));
            skipWhitespace();
            if (match('}')) {
                break;
            }
            if (!match(',')) {
                throw std::runtime_error("Expected ',' in JSON object");
            }
        }

        return object;
    }

    Array parseArray() {
        if (!match('[')) {
            throw std::runtime_error("Expected '[' at beginning of JSON array");
        }
        skipWhitespace();

        Array array;
        if (match(']')) {
            return array;
        }

        while (true) {
            array.push_back(parseValue());
            skipWhitespace();
            if (match(']')) {
                break;
            }
            if (!match(',')) {
                throw std::runtime_error("Expected ',' in JSON array");
            }
        }

        return array;
    }
};

std::string indentString(int depth, int indent) {
    return std::string(depth * indent, ' ');
}

std::string escapeString(const std::string& input) {
    std::ostringstream oss;
    oss << '"';
    for (char c : input) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
                } else {
                    oss << c;
                }
        }
    }
    oss << '"';
    return oss.str();
}

std::string toJsonStringInternal(const Value& value, int indent, int depth) {
    if (value.isPrimitive()) {
        const Primitive& primitive = value.asPrimitive();
        return std::visit(
            [&](const auto& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, std::nullptr_t>) {
                    return "null";
                } else if constexpr (std::is_same_v<T, bool>) {
                    return v ? "true" : "false";
                } else if constexpr (std::is_same_v<T, double>) {
                    std::ostringstream oss;
                    oss << v;
                    return oss.str();
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return escapeString(v);
                }
            },
            primitive);
    }

    if (value.isObject()) {
        const Object& object = value.asObject();
        if (object.empty()) {
            return "{}";
        }

        std::ostringstream oss;
        const bool pretty = indent > 0;
        oss << '{';
        bool first = true;
        for (const auto& [key, val] : object) {
            if (!first) {
                oss << (pretty ? ",\n" : ",");
            } else if (pretty) {
                oss << '\n';
            }
            first = false;
            if (pretty) {
                oss << indentString(depth + 1, indent);
            }
            oss << escapeString(key) << ':';
            if (pretty) {
                oss << ' ';
            }
            oss << toJsonStringInternal(val, indent, depth + 1);
        }
        if (pretty) {
            oss << '\n' << indentString(depth, indent);
        }
        oss << '}';
        return oss.str();
    }

    if (value.isArray()) {
        const Array& arr = value.asArray();
        if (arr.empty()) {
            return "[]";
        }

        std::ostringstream oss;
        const bool pretty = indent > 0;
        oss << '[';
        for (size_t i = 0; i < arr.size(); ++i) {
            if (i > 0) {
                oss << (pretty ? ",\n" : ",");
            } else if (pretty) {
                oss << '\n';
            }
            if (pretty) {
                oss << indentString(depth + 1, indent);
            }
            oss << toJsonStringInternal(arr[i], indent, depth + 1);
        }
        if (pretty) {
            oss << '\n' << indentString(depth, indent);
        }
        oss << ']';
        return oss.str();
    }

    return "null";
}

} // namespace

Value parseJson(const std::string& jsonString) {
    JsonParser parser(jsonString);
    return parser.parse();
}

Value parseJsonFromFile(const std::string& inputFile) {
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file: " + inputFile);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseJson(buffer.str());
}

std::string toJsonString(const Value& value, int indent) {
    return toJsonStringInternal(value, indent, 0);
}

Value loadJson(const std::string& filename) {
    return parseJsonFromFile(filename);
}

Value loadsJson(const std::string& jsonString) {
    return parseJson(jsonString);
}

std::string dumpsJson(const Value& value, int indent) {
    return toJsonString(value, indent);
}

void dumpJson(const Value& value, const std::string& filename, int indent) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + filename);
    }

    file << toJsonString(value, indent);
}

} // namespace serin

