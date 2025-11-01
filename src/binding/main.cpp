//
// Created by mohammad on 11/1/25.
//

// serin_bind.cpp
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/unordered_map.h>
#include <nanobind/stl/variant.h>
#include "serin.h"

namespace nb = nanobind;
using namespace serin;

NB_MODULE(serin, m) {
    nb::class_<Value>(m, "Value")
    .def(nb::init<>())
    .def("is_object", &Value::isObject)
    .def("is_array", &Value::isArray)
    .def("is_primitive", &Value::isPrimitive);

    m.def("value_loads_json", &loadsJson);
    m.def("value_dumps_json", &dumpsJson);
    m.def("value_loads_toon", &loadsToon);
    m.def("value_dumps_toon", &dumpsToon);
}
