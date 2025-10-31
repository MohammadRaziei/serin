#include "utils.h"

#include <cctype>

namespace serin {

std::string trim(std::string_view view) {
    size_t begin = 0;
    while (begin < view.size() && std::isspace(static_cast<unsigned char>(view[begin]))) {
        ++begin;
    }

    size_t end = view.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(view[end - 1]))) {
        --end;
    }

    return std::string(view.substr(begin, end - begin));
}

} // namespace serin

