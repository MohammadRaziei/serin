#pragma once

#include <string>
#include <string_view>

namespace serin {

// Removes leading and trailing ASCII whitespace characters from the given string view.
// Returns a new std::string containing the trimmed text.
std::string trim(std::string_view view);

} // namespace serin

