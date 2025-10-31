#include "serin.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace serin {

bool isPrimitive(const Value& value) { return value.isPrimitive(); }
bool isObject(const Value& value) { return value.isObject(); }
bool isArray(const Value& value) { return value.isArray(); }

} // namespace serin
