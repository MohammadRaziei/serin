#include "serin.h"
#include "utils.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

namespace serin {

namespace {

using serin::trim;

struct Line {
  int indent{};
  bool isListItem{};
  std::string
      text; // trimmed text (for list items this still contains the leading '-')
};

Primitive makePrimitiveNull() { return Primitive{std::nullptr_t{}}; }

bool looksInteger(const std::string &token, int base = 10) {
  if (token.empty()) {
    return false;
  }

  char *endPtr = nullptr;
  errno = 0;
  const long long value = std::strtoll(token.c_str(), &endPtr, base);
  (void)value;
  return endPtr && *endPtr == '\0' && endPtr != token.c_str() && errno == 0;
}

bool looksFloatingPoint(const std::string &token) {
  if (token.empty()) {
    return false;
  }

  char *endPtr = nullptr;
  errno = 0;
  const double numeric = std::strtod(token.c_str(), &endPtr);
  (void)numeric;
  return endPtr && *endPtr == '\0' && endPtr != token.c_str() && errno == 0;
}

Primitive parseScalarPrimitive(const std::string &token) {
  if (token.empty()) {
    return Primitive{std::string{}};
  }

  if (token == "null" || token == "Null" || token == "NULL" || token == "~") {
    return makePrimitiveNull();
  }
  if (token == "true" || token == "True" || token == "TRUE") {
    return Primitive{true};
  }
  if (token == "false" || token == "False" || token == "FALSE") {
    return Primitive{false};
  }

  if (looksInteger(token)) {
    errno = 0;
    return Primitive{static_cast<int64_t>(std::strtoll(token.c_str(), nullptr, 10))};
  }

  if (looksFloatingPoint(token)) {
    errno = 0;
    return Primitive{std::strtod(token.c_str(), nullptr)};
  }

  if (token.front() == '"' && token.back() == '"') {
    const std::string inner = token.substr(1, token.size() - 2);
    std::string result;
    result.reserve(inner.size());
    for (size_t i = 0; i < inner.size(); ++i) {
      char c = inner[i];
      if (c == '\\' && i + 1 < inner.size()) {
        char next = inner[++i];
        switch (next) {
        case 'n':
          result.push_back('\n');
          break;
        case 't':
          result.push_back('\t');
          break;
        case '\\':
          result.push_back('\\');
          break;
        case '"':
          result.push_back('"');
          break;
        default:
          result.push_back(next);
          break;
        }
      } else {
        result.push_back(c);
      }
    }
    return Primitive{result};
  }

  if (token.front() == '\'' && token.back() == '\'') {
    const std::string inner = token.substr(1, token.size() - 2);
    std::string result;
    result.reserve(inner.size());
    for (size_t i = 0; i < inner.size(); ++i) {
      char c = inner[i];
      if (c == '\'' && i + 1 < inner.size() && inner[i + 1] == '\'') {
        result.push_back('\'');
        ++i;
      } else {
        result.push_back(c);
      }
    }
    return Primitive{result};
  }

  return Primitive{token};
}

Value parseScalar(const std::string &token) {
  return Value(parseScalarPrimitive(token));
}

class YamlParser {
public:
  explicit YamlParser(std::vector<Line> lines) : lines_(std::move(lines)) {}

  Value parse() {
    if (lines_.empty()) {
      return Value(makePrimitiveNull());
    }
    return parseValue(lines_[0].indent);
  }

private:
  Value parseValue(int indent) {
    if (index_ >= lines_.size()) {
      return Value(makePrimitiveNull());
    }

    const Line &current = lines_[index_];
    if (current.indent > indent) {
      return parseValue(current.indent);
    }

    if (current.isListItem) {
      return parseSequence(current.indent);
    }

    if (current.text.find(':') == std::string::npos) {
      std::string scalarText = trim(current.text);
      ++index_;
      return parseScalar(scalarText);
    }

    return parseMapping(current.indent);
  }

  Value parseSequence(int indent) {
    Array result;
    while (index_ < lines_.size()) {
      Line line = lines_[index_];
      if (!line.isListItem || line.indent != indent) {
        break;
      }

      std::string content = trim(line.text.substr(1));
      ++index_;

      size_t nestedEnd = index_;
      while (nestedEnd < lines_.size() && lines_[nestedEnd].indent > indent) {
        ++nestedEnd;
      }

      std::vector<Line> nestedLines;
      if (!content.empty()) {
        const bool nestedIsList = !content.empty() && content[0] == '-';
        nestedLines.push_back(Line{indent + 2, nestedIsList, content});
      }
      nestedLines.insert(nestedLines.end(), lines_.begin() + index_,
                         lines_.begin() + nestedEnd);

      Value element;
      if (!nestedLines.empty()) {
        YamlParser nestedParser(std::move(nestedLines));
        element = nestedParser.parse();
      } else {
        element = Value(makePrimitiveNull());
      }

      result.push_back(element);
      index_ = nestedEnd;
    }
    return Value(result);
  }

  Value parseMapping(int indent) {
    Object result;
    while (index_ < lines_.size()) {
      Line line = lines_[index_];
      if (line.indent != indent || line.isListItem) {
        break;
      }

      const auto colonPos = line.text.find(':');
      if (colonPos == std::string::npos) {
        break;
      }

      std::string key = trim(std::string_view(line.text).substr(0, colonPos));
      std::string remainder =
          trim(std::string_view(line.text).substr(colonPos + 1));
      ++index_;

      if (!remainder.empty()) {
        result.emplace(std::move(key), parseScalar(remainder));
        continue;
      }

      if (index_ < lines_.size() && lines_[index_].indent > indent) {
        result.emplace(std::move(key), parseValue(lines_[index_].indent));
      } else {
        result.emplace(std::move(key), Value(makePrimitiveNull()));
      }
    }

    if (result.empty()) {
      return Value(makePrimitiveNull());
    }
    return Value(result);
  }

  std::vector<Line> lines_;
  size_t index_ = 0;
};

std::vector<Line> preprocess(const std::string &yamlString) {
  std::vector<Line> lines;
  std::istringstream stream(yamlString);
  std::string rawLine;
  while (std::getline(stream, rawLine)) {
    std::string_view view(rawLine);
    size_t commentPos = std::string::npos;
    bool inQuotes = false;
    char quoteChar = '\0';
    for (size_t i = 0; i < view.size(); ++i) {
      char c = view[i];
      if ((c == '"' || c == '\'') && (i == 0 || view[i - 1] != '\\')) {
        if (!inQuotes) {
          inQuotes = true;
          quoteChar = c;
        } else if (quoteChar == c) {
          inQuotes = false;
        }
      }
      if (!inQuotes && c == '#') {
        commentPos = i;
        break;
      }
    }
    if (commentPos != std::string::npos) {
      view = view.substr(0, commentPos);
    }

    size_t indent = 0;
    while (indent < view.size() && view[indent] == ' ') {
      ++indent;
    }

    std::string trimmed = trim(view.substr(indent));
    if (trimmed.empty()) {
      continue;
    }

    bool isList = !trimmed.empty() && trimmed[0] == '-';
    lines.push_back(Line{static_cast<int>(indent), isList, std::move(trimmed)});
  }
  return lines;
}

bool needsQuoting(const std::string &value) {
  if (value.empty()) {
    return true;
  }

  if (std::isspace(static_cast<unsigned char>(value.front())) ||
      std::isspace(static_cast<unsigned char>(value.back()))) {
    return true;
  }

  for (char c : value) {
    if (c == '\n' || c == '\t' || c == '\r') {
      return true;
    }
  }

  const std::string lower = [&value]() {
    std::string tmp = value;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return tmp;
  }();

  if (lower == "null" || lower == "true" || lower == "false" || lower == "~") {
    return true;
  }

  if (looksInteger(value) || looksFloatingPoint(value)) {
    return true;
  }

  if (value.front() == '-' || value.front() == ':' || value.front() == '#' ||
      value.front() == '?' || value.front() == '@' || value.front() == '&' ||
      value.front() == '*' || value.front() == '!' || value.front() == '%' ||
      value.front() == '|') {
    return true;
  }

  for (char c : value) {
    switch (c) {
    case ':':
      return true;
    case '{':
    case '}':
    case '[':
    case ']':
    case ',':
      return true;
    default:
      break;
    }
  }

  return false;
}

std::string encodeScalar(const Primitive &primitive) {
  // Use Primitive::asString() which already uses yyjson for number serialization
  std::string result = primitive.asString();
  
  // For strings, we still need to handle YAML-specific quoting
  if (primitive.isString() && needsQuoting(result)) {
    std::string escaped;
    escaped.reserve(result.size() + 2);
    escaped.push_back('"');
    for (char c : result) {
      switch (c) {
      case '\n':
        escaped += "\\n";
        break;
      case '\t':
        escaped += "\\t";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\\':
        escaped += "\\\\";
        break;
      default:
        escaped.push_back(c);
        break;
      }
    }
    escaped.push_back('"');
    return escaped;
  }
  
  return result;
}

void dumpValue(const Value &value, int indent, int indentStep,
               std::string &out) {
  auto indentString = [indent]() {
    return std::string(static_cast<size_t>(indent), ' ');
  };

  if (value.isPrimitive()) {
    out += indentString();
    out += encodeScalar(value.asPrimitive());
    out += '\n';
    return;
  }

  if (value.isArray()) {
    const auto &array = value.asArray();
    if (array.empty()) {
      out += indentString();
      out += "[]\n";
      return;
    }

    for (const auto &element : array) {
      out += indentString();
      out += "-";
      if (element.isPrimitive()) {
        out.push_back(' ');
        out += encodeScalar(element.asPrimitive());
        out += '\n';
        continue;
      }

      if (element.isObject()) {
        const auto &object = element.asObject();
        if (object.empty()) {
          out += " {}\n";
          continue;
        }

        auto it = object.begin();
        const auto end = object.end();
        out.push_back(' ');
        out += it->first;
        out += ":";
        if (it->second.isPrimitive()) {
          out.push_back(' ');
          out += encodeScalar(it->second.asPrimitive());
          out += '\n';
        } else {
          out += '\n';
          dumpValue(it->second, indent + indentStep, indentStep, out);
        }
        ++it;
        for (; it != end; ++it) {
          out += std::string(static_cast<size_t>(indent + indentStep), ' ');
          out += it->first;
          out += ":";
          if (it->second.isPrimitive()) {
            out.push_back(' ');
            out += encodeScalar(it->second.asPrimitive());
            out += '\n';
          } else {
            out += '\n';
            dumpValue(it->second, indent + indentStep, indentStep, out);
          }
        }
        continue;
      }

      out += '\n';
      dumpValue(element, indent + indentStep, indentStep, out);
    }
    return;
  }

  const auto &object = value.asObject();
  if (object.empty()) {
    out += indentString();
    out += "{}\n";
    return;
  }

  for (const auto &[key, element] : object) {
    out += indentString();
    out += key;
    out += ":";
    if (element.isPrimitive()) {
      out.push_back(' ');
      out += encodeScalar(element.asPrimitive());
      out += '\n';
    } else {
      out += '\n';
      dumpValue(element, indent + indentStep, indentStep, out);
    }
  }
}

} // namespace

Value loadYaml(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open YAML file: " + filename);
  }
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return loadsYaml(buffer.str());
}

Value loadsYaml(const std::string &yamlString) {
  auto lines = preprocess(yamlString);
  YamlParser parser(std::move(lines));
  return parser.parse();
}

std::string dumpsYaml(const Value &value, int indent [[maybe_unused]]) {
  std::string output;
  const int indentStep = indent > 0 ? indent : 2;
  dumpValue(value, 0, indentStep, output);
  if (!output.empty() && output.back() == '\n') {
    output.pop_back();
  }
  return output;
}

void dumpYaml(const Value &value, const std::string &filename, int indent) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open YAML file for writing: " + filename);
  }
  file << dumpsYaml(value, indent);
}

} // namespace serin
