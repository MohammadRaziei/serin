#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "serin.h"

#include <variant>

namespace {

const serin::Object& expectObject(const serin::Value& value) {
    REQUIRE(value.isObject());
    return value.asObject();
}

const serin::Array& expectArray(const serin::Value& value) {
    REQUIRE(value.isArray());
    return value.asArray();
}

const std::string& expectString(const serin::Value& value) {
    const auto& primitive = value.asPrimitive();
    REQUIRE(std::holds_alternative<std::string>(primitive));
    return std::get<std::string>(primitive);
}

std::string trim(const std::string& input) {
    const auto first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = input.find_last_not_of(" \t\r\n");
    return input.substr(first, last - first + 1);
}

double expectNumber(const serin::Value& value) {
    const auto& primitive = value.asPrimitive();
    if (std::holds_alternative<double>(primitive)) {
        return std::get<double>(primitive);
    }
    if (std::holds_alternative<int64_t>(primitive)) {
        return static_cast<double>(std::get<int64_t>(primitive));
    }
    FAIL_CHECK("Primitive is not numeric");
    return 0.0;
}

bool expectBool(const serin::Value& value) {
    const auto& primitive = value.asPrimitive();
    REQUIRE(std::holds_alternative<bool>(primitive));
    return std::get<bool>(primitive);
}

void checkSample1User(const serin::Value& value) {
    const auto& obj = expectObject(value);
    CHECK_EQ(expectString(obj.at("name")), "Alice");
    CHECK_EQ(expectNumber(obj.at("age")), doctest::Approx(30.0));
    CHECK(expectBool(obj.at("active")));

    const auto& tags = expectArray(obj.at("tags"));
    REQUIRE_EQ(tags.size(), 3);
    CHECK_EQ(expectString(tags[0]), "programming");
    CHECK_EQ(expectString(tags[1]), "c++");
    CHECK_EQ(expectString(tags[2]), "serialization");
}

void checkSample2Users(const serin::Value& value) {
    const auto& obj = expectObject(value);
    const auto& users = expectArray(obj.at("users"));
    REQUIRE_EQ(users.size(), 2);

    const auto& user1 = expectObject(users[0]);
    CHECK_EQ(expectNumber(user1.at("id")), doctest::Approx(1.0));
    CHECK_EQ(expectString(user1.at("name")), "Alice");
    CHECK_EQ(expectString(user1.at("role")), "admin");

    const auto& user2 = expectObject(users[1]);
    CHECK_EQ(expectNumber(user2.at("id")), doctest::Approx(2.0));
    CHECK_EQ(expectString(user2.at("name")), "Bob");
    CHECK_EQ(expectString(user2.at("role")), "user");
}

void checkSample3Nested(const serin::Value& value) {
    const auto& obj = expectObject(value);
    const auto& order = expectObject(obj.at("order"));

    CHECK_EQ(expectString(order.at("id")), "ORD-12345");
    CHECK_EQ(expectString(order.at("status")), "completed");

    const auto& customer = expectObject(order.at("customer"));
    CHECK_EQ(expectString(customer.at("name")), "John Doe");
    CHECK_EQ(expectString(customer.at("email")), "john@example.com");

    const auto& items = expectArray(order.at("items"));
    REQUIRE_EQ(items.size(), 2);

    const auto& item1 = expectObject(items[0]);
    CHECK_EQ(expectString(item1.at("product")), "Book");
    CHECK_EQ(expectNumber(item1.at("quantity")), doctest::Approx(2.0));
    CHECK_EQ(expectNumber(item1.at("price")), doctest::Approx(15.0));

    const auto& item2 = expectObject(items[1]);
    CHECK_EQ(expectString(item2.at("product")), "Pen");
    CHECK_EQ(expectNumber(item2.at("quantity")), doctest::Approx(5.0));
    CHECK_EQ(expectNumber(item2.at("price")), doctest::Approx(2.5));
}

} // namespace

template <typename DumpFn, typename LoadFn>
void expectConversion(const serin::Value& source,
                      DumpFn&& dumps,
                      LoadFn&& loads,
                      void (*validator)(const serin::Value&)) {
    const auto serialized = dumps(source);
    const auto converted = loads(serialized);
    validator(converted);
}

TEST_CASE("Sample 1 stays consistent across formats") {
    const auto jsonValue = serin::loadJson("tests/data/sample1_user.json");
    const auto yamlValue = serin::loadYaml("tests/data/sample1_user.yaml");
    const auto toonValue = serin::loadToon("tests/data/sample1_user.toon");
    const auto& toonText = expectString(toonValue);

    checkSample1User(jsonValue);
    checkSample1User(yamlValue);

    const auto canonicalToon = serin::dumpsToon(jsonValue);
    CHECK_EQ(trim(toonText), trim(canonicalToon));

    SUBCASE("JSON dumps can be parsed back") {
        serin::Object data;
        data["name"] = serin::Value("Alice");
        data["age"] = serin::Value(30.0);
        data["active"] = serin::Value(true);

        serin::Array tags;
        tags.push_back(serin::Value("programming"));
        tags.push_back(serin::Value("c++"));
        tags.push_back(serin::Value("serialization"));
        data["tags"] = serin::Value(tags);

        const serin::Value value(data);
        checkSample1User(serin::loadsJson(serin::dumpsJson(value)));
    }

    SUBCASE("YAML dumps can be parsed back") {
        serin::Object data;
        data["name"] = serin::Value("Alice");
        data["age"] = serin::Value(30.0);
        data["active"] = serin::Value(true);

        serin::Array tags;
        tags.push_back(serin::Value("programming"));
        tags.push_back(serin::Value("c++"));
        tags.push_back(serin::Value("serialization"));
        data["tags"] = serin::Value(tags);

        const serin::Value value(data);
        checkSample1User(serin::loadsYaml(serin::dumpsYaml(value)));
    }

    SUBCASE("TOON dumps can be parsed back") {
        serin::Object data;
        data["name"] = serin::Value("Alice");
        data["age"] = serin::Value(30.0);
        data["active"] = serin::Value(true);

        serin::Array tags;
        tags.push_back(serin::Value("programming"));
        tags.push_back(serin::Value("c++"));
        tags.push_back(serin::Value("serialization"));
        data["tags"] = serin::Value(tags);

        const serin::Value value(data);
        const auto toon = serin::dumpsToon(value);
        const auto parsed = serin::loadsToon(toon);
        CHECK_EQ(trim(expectString(parsed)), trim(toon));
    }

    SUBCASE("JSON to YAML conversion") {
        expectConversion(jsonValue,
                         [](const serin::Value& value) { return serin::dumpsYaml(value); },
                         serin::loadsYaml,
                         checkSample1User);
    }

    SUBCASE("JSON to TOON conversion") {
        const auto generated = serin::dumpsToon(jsonValue);
        CHECK_EQ(trim(generated), trim(toonText));
    }

    SUBCASE("YAML to JSON conversion") {
        expectConversion(yamlValue,
                         [](const serin::Value& value) { return serin::dumpsJson(value); },
                         serin::loadsJson,
                         checkSample1User);
    }

    SUBCASE("YAML to TOON conversion") {
        const auto generated = serin::dumpsToon(yamlValue);
        CHECK_EQ(trim(generated), trim(toonText));
    }

    SUBCASE("TOON to JSON conversion") {
        const auto jsonText = serin::dumpsJson(toonValue);
        const auto parsed = serin::loadsJson(jsonText);
        CHECK_EQ(expectString(parsed), toonText);
    }

    SUBCASE("TOON to YAML conversion") {
        const auto yamlText = serin::dumpsYaml(toonValue);
        CHECK(yamlText.find("\\n") != std::string::npos);
        CHECK_NOTHROW(serin::loadsYaml(yamlText));
    }
}

TEST_CASE("Sample 2 stays consistent across formats") {
    const auto jsonValue = serin::loadJson("tests/data/sample2_users.json");
    const auto yamlValue = serin::loadYaml("tests/data/sample2_users.yaml");
    const auto toonValue = serin::loadToon("tests/data/sample2_users.toon");
    const auto& toonText = expectString(toonValue);

    checkSample2Users(jsonValue);
    checkSample2Users(yamlValue);

    const auto canonicalToon = serin::dumpsToon(jsonValue);
    CHECK_EQ(trim(toonText), trim(canonicalToon));

    SUBCASE("JSON to YAML conversion") {
        expectConversion(jsonValue,
                         [](const serin::Value& value) { return serin::dumpsYaml(value); },
                         serin::loadsYaml,
                         checkSample2Users);
    }

    SUBCASE("JSON to TOON conversion") {
        const auto generated = serin::dumpsToon(jsonValue);
        CHECK_EQ(trim(generated), trim(toonText));
    }

    SUBCASE("YAML to JSON conversion") {
        expectConversion(yamlValue,
                         [](const serin::Value& value) { return serin::dumpsJson(value); },
                         serin::loadsJson,
                         checkSample2Users);
    }

    SUBCASE("YAML to TOON conversion") {
        const auto generated = serin::dumpsToon(yamlValue);
        CHECK_EQ(trim(generated), trim(toonText));
    }

    SUBCASE("TOON to JSON conversion") {
        const auto jsonText = serin::dumpsJson(toonValue);
        const auto parsed = serin::loadsJson(jsonText);
        CHECK_EQ(expectString(parsed), toonText);
    }

    SUBCASE("TOON to YAML conversion") {
        const auto yamlText = serin::dumpsYaml(toonValue);
        CHECK(yamlText.find("\\n") != std::string::npos);
        CHECK_NOTHROW(serin::loadsYaml(yamlText));
    }
}

TEST_CASE("Sample 3 stays consistent across formats") {
    const auto jsonValue = serin::loadJson("tests/data/sample3_nested.json");
    const auto yamlValue = serin::loadYaml("tests/data/sample3_nested.yaml");
    const auto toonValue = serin::loadToon("tests/data/sample3_nested.toon");
    const auto& toonText = expectString(toonValue);

    checkSample3Nested(jsonValue);
    checkSample3Nested(yamlValue);

    const auto canonicalToon = serin::dumpsToon(jsonValue);
    CHECK_EQ(trim(toonText), trim(canonicalToon));

    SUBCASE("JSON to YAML conversion") {
        expectConversion(jsonValue,
                         [](const serin::Value& value) { return serin::dumpsYaml(value); },
                         serin::loadsYaml,
                         checkSample3Nested);
    }

    SUBCASE("JSON to TOON conversion") {
        const auto generated = serin::dumpsToon(jsonValue);
        CHECK_EQ(trim(generated), trim(toonText));
    }

    SUBCASE("YAML to JSON conversion") {
        expectConversion(yamlValue,
                         [](const serin::Value& value) { return serin::dumpsJson(value); },
                         serin::loadsJson,
                         checkSample3Nested);
    }

    SUBCASE("YAML to TOON conversion") {
        const auto generated = serin::dumpsToon(yamlValue);
        CHECK_EQ(trim(generated), trim(toonText));
    }

    SUBCASE("TOON to JSON conversion") {
        const auto jsonText = serin::dumpsJson(toonValue);
        const auto parsed = serin::loadsJson(jsonText);
        CHECK_EQ(expectString(parsed), toonText);
    }

    SUBCASE("TOON to YAML conversion") {
        const auto yamlText = serin::dumpsYaml(toonValue);
        CHECK(yamlText.find("\\n") != std::string::npos);
        CHECK_NOTHROW(serin::loadsYaml(yamlText));
    }
}

TEST_CASE("Toon options customize formatting supports alternate delimiters") {
    serin::Object obj;
    obj["name"] = serin::Value("Alice");

    serin::Array tags;
    tags.emplace_back(serin::Value("red"));
    tags.emplace_back(serin::Value("blue"));
    obj["tags"] = serin::Value(tags);

    serin::ToonOptions options;
    options.setIndent(4).setDelimiter(serin::Delimiter::Pipe);

    auto toon = serin::dumpsToon(serin::Value(obj), options);
    CHECK(toon.find("tags[2]: red|blue") != std::string::npos);
    CHECK(toon.find("name: Alice") != std::string::npos);
}
