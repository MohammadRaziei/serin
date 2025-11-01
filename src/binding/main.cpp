//
// Created by mohammad on 11/1/25.
//

// serin_bind.cpp
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/unordered_map.h>
#include <nanobind/stl/variant.h>
#include "serin.h"


#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)


namespace nb = nanobind;


serin::Value dict2value(nb::handle obj) {
    if (obj.is_none()) {
        return Value(nullptr);
    } else if (nb::isinstance<nb::bool_>(obj)) {
        return Value(static_cast<bool>(nb::cast<bool>(obj)));
    } else if (nb::isinstance<nb::int_>(obj)) {
        return Value(static_cast<int64_t>(nb::cast<int64_t>(obj)));
    } else if (nb::isinstance<nb::float_>(obj)) {
        return Value(static_cast<double>(nb::cast<double>(obj)));
    } else if (nb::isinstance<nb::str>(obj)) {
        return Value(nb::cast<std::string>(obj));
    } else if (nb::isinstance<nb::list>(obj)) {
        serin::Array arr;
        nb::list pylist = nb::cast<nb::list>(obj);
        for (auto item : pylist)
            arr.push_back(dict2value(item));
        return Value(arr);
    } else if (nb::isinstance<nb::dict>(obj)) {
        serin::Object map;
        nb::dict d = nb::cast<nb::dict>(obj);
        for (auto [k, v] : d) {
            map[nb::cast<std::string>(k)] = dict2value(v);
        }
        return Value(map);
    }
    throw std::runtime_error("Unsupported Python type for serin::Value conversion");
}

nb::object value2dict(const serin::Value& val) {
    if (val.isPrimitive()) {
        const auto& p = val.asPrimitive();
        return std::visit([](auto&& arg) -> nb::object {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) return nb::none();
            else return nb::cast(arg);
        }, p);
    } else if (val.isArray()) {
        nb::list out;
        for (const auto& e : val.asArray())
            out.append(value2dict(e));
        return std::move(out);
    } else if (val.isObject()) {
        nb::dict out;
        for (const auto& kv : val.asObject())
            out[nb::cast(kv.first)] = value2dict(kv.second);
        return std::move(out);
    }
    throw std::runtime_error("Unknown serin::Value type");
}


template<typename Func, typename... Args>
std::string dumpsDict(Func f, const serin::Object& dict, Args&&... args) {
    serin::Value v(dict);
    return f(v, std::forward<Args>(args)...);
}

template<typename Func, typename... Args>
std::string dumpDict(Func f, const serin::Object& dict, Args&&... args) {
    serin::Value v(dict);
    return f(v, std::forward<Args>(args)...);

// loadsXDict : str -> dict
template<typename Func, typename... Args>
serin::Object loadsDict(Func f, const std::string& s, Args&&... args) {
    serin::Value v = f(s, std::forward<Args>(args)...);
    return v.asObject();
}



NB_MODULE(NB_MODULE_NAME, m) {
    nb::class_<serin::Value>(m, "Value")
    .def(nb::init<>())
    .def("is_object", &serin::Value::isObject)
    .def("is_array", &serin::Value::isArray)
    .def("is_primitive", &serin::Value::isPrimitive);

    m.def("value_loads_json", &serin::loadsJson);
    m.def("value_dumps_json", &serin::dumpsJson);
    m.def("value_loads_toon", &serin::loadsToon);
    m.def("value_dumps_toon", &serin::dumpsToon);

    m.def("loads_json", &serin::loadsJson)

    #ifdef VERSION_INFO
        m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
    #else
        m.attr("__version__") = "dev";
    #endif
}
