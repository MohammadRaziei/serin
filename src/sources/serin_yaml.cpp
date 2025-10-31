//#include "serin.h"
//#include "utils.h"
//
//#include <cctype>
//#include <cstdlib>
//#include <fstream>
//#include <sstream>
//#include <stdexcept>
//#include <string_view>
//#include <type_traits>
//#include <utility>
//
//namespace serin {
//
//namespace {
//
//using serin::trim;
//
//struct Line {
//    int indent{};
//    bool isListItem{};
//    std::string text; // trimmed text (for list items this still contains the leading '-')
//};
//
//Primitive makePrimitiveNull() {
//    return Primitive{std::nullptr_t{}};
//}
//
//Primitive parseScalarPrimitive(const std::string& token) {
//    if (token.empty()) {
//        return Primitive{std::string{}};
//    }
//
//    if (token == "null" || token == "Null" || token == "NULL" || token == "~") {
//        return makePrimitiveNull();
//    }
//    if (token == "true" || token == "True" || token == "TRUE") {
//        return Primitive{true};
//    }
//    if (token == "false" || token == "False" || token == "FALSE") {
//        return Primitive{false};
//    }
//
//    char* endPtr = nullptr;
//    const double numeric = std::strtod(token.c_str(), &endPtr);
//    if (endPtr && *endPtr == '\0' && endPtr != token.c_str()) {
//        return Primitive{numeric};
//    }
//
//    if (token.front() == '"' && token.back() == '"') {
//        const std::string inner = token.substr(1, token.size() - 2);
//        std::string result;
//        result.reserve(inner.size());
//        for (size_t i = 0; i < inner.size(); ++i) {
//            char c = inner[i];
//            if (c == '\\' && i + 1 < inner.size()) {
//                char next = inner[++i];
//                switch (next) {
//                case 'n': result.push_back('\n'); break;
//                case 't': result.push_back('\t'); break;
//                case '\\': result.push_back('\\'); break;
//                case '"': result.push_back('"'); break;
//                default:
//                    result.push_back(next);
//                    break;
//                }
//            } else {
//                result.push_back(c);
//            }
//        }
//        return Primitive{result};
//    }
//
//    if (token.front() == '\'' && token.back() == '\'') {
//        const std::string inner = token.substr(1, token.size() - 2);
//        std::string result;
//        result.reserve(inner.size());
//        for (size_t i = 0; i < inner.size(); ++i) {
//            char c = inner[i];
//            if (c == '\'' && i + 1 < inner.size() && inner[i + 1] == '\'') {
//                result.push_back('\'');
//                ++i;
//            } else {
//                result.push_back(c);
//            }
//        }
//        return Primitive{result};
//    }
//
//    return Primitive{token};
//}
//
//Value parseScalar(const std::string& token) {
//    return Value(parseScalarPrimitive(token));
//}
//
//class YamlParser {
//public:
//    explicit YamlParser(std::vector<Line> lines) : lines_(std::move(lines)) {}
//
//    Value parse() {
//        if (lines_.empty()) {
//            return Value(makePrimitiveNull());
//        }
//        return parseValue(lines_[0].indent);
//    }
//
//private:
//    Value parseValue(int indent) {
//        if (index_ >= lines_.size()) {
//            return Value(makePrimitiveNull());
//        }
//
//        const Line& current = lines_[index_];
//        if (current.indent > indent) {
//            return parseValue(current.indent);
//        }
//
//        if (current.isListItem) {
//            return parseSequence(current.indent);
//        }
//
//        if (current.text.find(':') == std::string::npos) {
//            std::string scalarText = trim(current.text);
//            ++index_;
//            return parseScalar(scalarText);
//        }
//
//        return parseMapping(current.indent);
//    }
//
//    Value parseSequence(int indent) {
//        Array result;
//        while (index_ < lines_.size()) {
//            Line line = lines_[index_];
//            if (!line.isListItem || line.indent != indent) {
//                break;
//            }
//
//            std::string content = trim(line.text.substr(1));
//            ++index_;
//
//            size_t nestedEnd = index_;
//            while (nestedEnd < lines_.size() && lines_[nestedEnd].indent > indent) {
//                ++nestedEnd;
//            }
//
//            std::vector<Line> nestedLines;
//            if (!content.empty()) {
//                const bool nestedIsList = !content.empty() && content[0] == '-';
//                nestedLines.push_back(Line{indent + 2, nestedIsList, content});
//            }
//            nestedLines.insert(nestedLines.end(), lines_.begin() + index_, lines_.begin() + nestedEnd);
//
//            Value element;
//            if (!nestedLines.empty()) {
//                YamlParser nestedParser(std::move(nestedLines));
//                element = nestedParser.parse();
//            } else {
//                element = Value(makePrimitiveNull());
//            }
//
//            result.push_back(element);
//            index_ = nestedEnd;
//        }
//        return Value(result);
//    }
//
//    Value parseMapping(int indent) {
//        Object result;
//        while (index_ < lines_.size()) {
//            Line line = lines_[index_];
//            if (line.indent != indent || line.isListItem) {
//                break;
//            }
//
//            const auto colonPos = line.text.find(':');
//            if (colonPos == std::string::npos) {
//                break;
//            }
//
//            std::string key = trim(std::string_view(line.text).substr(0, colonPos));
//            std::string remainder = trim(std::string_view(line.text).substr(colonPos + 1));
//            ++index_;
//
//            if (!remainder.empty()) {
//                result.emplace(std::move(key), parseScalar(remainder));
//                continue;
//            }
//
//            if (index_ < lines_.size() && lines_[index_].indent > indent) {
//                result.emplace(std::move(key), parseValue(lines_[index_].indent));
//            } else {
//                result.emplace(std::move(key), Value(makePrimitiveNull()));
//            }
//        }
//
//        if (result.empty()) {
//            return Value(makePrimitiveNull());
//        }
//        return Value(result);
//    }
//
//    std::vector<Line> lines_;
//    size_t index_ = 0;
//};
//
//std::vector<Line> preprocess(const std::string& yamlString) {
//    std::vector<Line> lines;
//    std::istringstream stream(yamlString);
//    std::string rawLine;
//    while (std::getline(stream, rawLine)) {
//        std::string_view view(rawLine);
//        size_t commentPos = std::string::npos;
//        bool inQuotes = false;
//        char quoteChar = '\0';
//        for (size_t i = 0; i < view.size(); ++i) {
//            char c = view[i];
//            if ((c == '"' || c == '\'') && (i == 0 || view[i - 1] != '\\')) {
//                if (!inQuotes) {
//                    inQuotes = true;
//                    quoteChar = c;
//                } else if (quoteChar == c) {
//                    inQuotes = false;
//                }
//            }
//            if (!inQuotes && c == '#') {
//                commentPos = i;
//                break;
//            }
//        }
//        if (commentPos != std::string::npos) {
//            view = view.substr(0, commentPos);
//        }
//
//        size_t indent = 0;
//        while (indent < view.size() && view[indent] == ' ') {
//            ++indent;
//        }
//
//        std::string trimmed = trim(view.substr(indent));
//        if (trimmed.empty()) {
//            continue;
//        }
//
//        bool isList = !trimmed.empty() && trimmed[0] == '-';
//        lines.push_back(Line{static_cast<int>(indent), isList, std::move(trimmed)});
//    }
//    return lines;
//}
//
//std::string encodeScalar(const Primitive& primitive) {
//    return std::visit(
//        [](const auto& value) -> std::string {
//            using T = std::decay_t<decltype(value)>;
//            if constexpr (std::is_same_v<T, std::nullptr_t>) {
//                return "null";
//            } else if constexpr (std::is_same_v<T, bool>) {
//                return value ? "true" : "false";
//            } else if constexpr (std::is_same_v<T, double>) {
//                std::ostringstream oss;
//                oss << value;
//                return oss.str();
//            } else { // std::string
//                const bool needsQuotes = value.find_first_of("\n:#-{}[]\"'") != std::string::npos || value.empty();
//                if (!needsQuotes) {
//                    return value;
//                }
//                std::string escaped;
//                escaped.reserve(value.size() + 2);
//                escaped.push_back('"');
//                for (char c : value) {
//                    switch (c) {
//                    case '\n': escaped += "\\n"; break;
//                    case '\t': escaped += "\\t"; break;
//                    case '"': escaped += "\\\""; break;
//                    case '\\': escaped += "\\\\"; break;
//                    default: escaped.push_back(c); break;
//                    }
//                }
//                escaped.push_back('"');
//                return escaped;
//            }
//        },
//        primitive);
//}
//
//void dumpValue(const Value& value, int indent, std::string& out) {
//    auto indentString = [indent]() {
//        return std::string(static_cast<size_t>(indent), ' ');
//    };
//
//    if (value.isPrimitive()) {
//        out += indentString();
//        out += encodeScalar(value.asPrimitive());
//        out += '\n';
//        return;
//    }
//
//    if (value.isArray()) {
//        const auto& array = value.asArray();
//        if (array.empty()) {
//            out += indentString();
//            out += "[]\n";
//            return;
//        }
//
//        for (const auto& element : array) {
//            out += indentString();
//            out += "-";
//            if (element.isPrimitive()) {
//                out.push_back(' ');
//                out += encodeScalar(element.asPrimitive());
//                out += '\n';
//            } else {
//                out += '\n';
//                dumpValue(element, indent + 2, out);
//            }
//        }
//        return;
//    }
//
//    const auto& object = value.asObject();
//    if (object.empty()) {
//        out += indentString();
//        out += "{}\n";
//        return;
//    }
//
//    for (const auto& [key, element] : object) {
//        out += indentString();
//        out += key;
//        out += ":";
//        if (element.isPrimitive()) {
//            out.push_back(' ');
//            out += encodeScalar(element.asPrimitive());
//            out += '\n';
//        } else {
//            out += '\n';
//            dumpValue(element, indent + 2, out);
//        }
//    }
//}
//
//} // namespace
//
//Value loadYaml(const std::string& filename) {
//    std::ifstream file(filename);
//    if (!file.is_open()) {
//        throw std::runtime_error("Cannot open YAML file: " + filename);
//    }
//    std::ostringstream buffer;
//    buffer << file.rdbuf();
//    return loadsYaml(buffer.str());
//}
//
//Value loadsYaml(const std::string& yamlString) {
//    auto lines = preprocess(yamlString);
//    YamlParser parser(std::move(lines));
//    return parser.parse();
//}
//
//std::string dumpsYaml(const Value& value, int indent) {
//    std::string output;
//    dumpValue(value, 0, output);
//    if (!output.empty() && output.back() == '\n') {
//        output.pop_back();
//    }
//    return output;
//}
//
//void dumpYaml(const Value& value, const std::string& filename, int indent) {
//    std::ofstream file(filename);
//    if (!file.is_open()) {
//        throw std::runtime_error("Cannot open YAML file for writing: " + filename);
//    }
//    file << dumpsYaml(value, indent);
//}
//
//} // namespace serin
