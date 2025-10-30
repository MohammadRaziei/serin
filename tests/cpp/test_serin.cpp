#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "serin.h"

#include <fstream>
#include <sstream>

TEST_CASE("Basic JSON serialization") {
    SUBCASE("Load JSON from file") {
        auto value = serin::loadJson("tests/data/sample1_user.json");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        CHECK(obj["name"].asPrimitive() == "Alice");
        CHECK(obj["age"].asPrimitive() == 30.0);
        CHECK(obj["active"].asPrimitive() == true);
        
        auto tags = obj["tags"].asArray();
        CHECK(tags.size() == 3);
        CHECK(tags[0].asPrimitive() == "programming");
        CHECK(tags[1].asPrimitive() == "c++");
        CHECK(tags[2].asPrimitive() == "serialization");
    }
    
    SUBCASE("Dump JSON to string") {
        serin::Object data;
        data["name"] = serin::Value("Alice");
        data["age"] = serin::Value(30.0);
        data["active"] = serin::Value(true);
        
        serin::Array tags;
        tags.push_back(serin::Value("programming"));
        tags.push_back(serin::Value("c++"));
        tags.push_back(serin::Value("serialization"));
        data["tags"] = serin::Value(tags);
        
        serin::Value value(data);
        std::string json = serin::dumpsJson(value);
        
        CHECK(json.find("\"name\": \"Alice\"") != std::string::npos);
        CHECK(json.find("\"age\": 30") != std::string::npos);
        CHECK(json.find("\"active\": true") != std::string::npos);
    }
}

TEST_CASE("Basic TOON serialization") {
    SUBCASE("Load TOON from file") {
        auto value = serin::loadToon("tests/data/sample1_user.toon");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        CHECK(obj["name"].asPrimitive() == "Alice");
        CHECK(obj["age"].asPrimitive() == 30.0);
        CHECK(obj["active"].asPrimitive() == true);
        
        auto tags = obj["tags"].asArray();
        CHECK(tags.size() == 3);
        CHECK(tags[0].asPrimitive() == "programming");
        CHECK(tags[1].asPrimitive() == "c++");
        CHECK(tags[2].asPrimitive() == "serialization");
    }
    
    SUBCASE("Dump TOON to string") {
        serin::Object data;
        data["name"] = serin::Value("Alice");
        data["age"] = serin::Value(30.0);
        data["active"] = serin::Value(true);
        
        serin::Array tags;
        tags.push_back(serin::Value("programming"));
        tags.push_back(serin::Value("c++"));
        tags.push_back(serin::Value("serialization"));
        data["tags"] = serin::Value(tags);
        
        serin::Value value(data);
        std::string toon = serin::dumpsToon(value);
        
        CHECK(toon.find("name: Alice") != std::string::npos);
        CHECK(toon.find("age: 30") != std::string::npos);
        CHECK(toon.find("active: true") != std::string::npos);
        CHECK(toon.find("tags[3]: programming,c++,serialization") != std::string::npos);
    }
}

TEST_CASE("Basic YAML serialization") {
    SUBCASE("Load YAML from file") {
        auto value = serin::loadYaml("tests/data/sample1_user.yaml");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        CHECK(obj["name"].asPrimitive() == "Alice");
        CHECK(obj["age"].asPrimitive() == 30.0);
        CHECK(obj["active"].asPrimitive() == true);
        
        auto tags = obj["tags"].asArray();
        CHECK(tags.size() == 3);
        CHECK(tags[0].asPrimitive() == "programming");
        CHECK(tags[1].asPrimitive() == "c++");
        CHECK(tags[2].asPrimitive() == "serialization");
    }
    
    SUBCASE("Dump YAML to string") {
        serin::Object data;
        data["name"] = serin::Value("Alice");
        data["age"] = serin::Value(30.0);
        data["active"] = serin::Value(true);
        
        serin::Array tags;
        tags.push_back(serin::Value("programming"));
        tags.push_back(serin::Value("c++"));
        tags.push_back(serin::Value("serialization"));
        data["tags"] = serin::Value(tags);
        
        serin::Value value(data);
        std::string yaml = serin::dumpsYaml(value);
        
        CHECK(yaml.find("name: Alice") != std::string::npos);
        CHECK(yaml.find("age: 30") != std::string::npos);
        CHECK(yaml.find("active: true") != std::string::npos);
        CHECK(yaml.find("- programming") != std::string::npos);
        CHECK(yaml.find("- c++") != std::string::npos);
        CHECK(yaml.find("- serialization") != std::string::npos);
    }
}

TEST_CASE("Tabular data serialization") {
    SUBCASE("JSON tabular data") {
        auto value = serin::loadJson("tests/data/sample2_users.json");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        auto users = obj["users"].asArray();
        CHECK(users.size() == 2);
        
        auto user1 = users[0].asObject();
        CHECK(user1["id"].asPrimitive() == 1.0);
        CHECK(user1["name"].asPrimitive() == "Alice");
        CHECK(user1["role"].asPrimitive() == "admin");
        
        auto user2 = users[1].asObject();
        CHECK(user2["id"].asPrimitive() == 2.0);
        CHECK(user2["name"].asPrimitive() == "Bob");
        CHECK(user2["role"].asPrimitive() == "user");
    }
    
    SUBCASE("TOON tabular data") {
        auto value = serin::loadToon("tests/data/sample2_users.toon");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        auto users = obj["users"].asArray();
        CHECK(users.size() == 2);
        
        auto user1 = users[0].asObject();
        CHECK(user1["id"].asPrimitive() == 1.0);
        CHECK(user1["name"].asPrimitive() == "Alice");
        CHECK(user1["role"].asPrimitive() == "admin");
        
        auto user2 = users[1].asObject();
        CHECK(user2["id"].asPrimitive() == 2.0);
        CHECK(user2["name"].asPrimitive() == "Bob");
        CHECK(user2["role"].asPrimitive() == "user");
    }
    
    SUBCASE("YAML tabular data") {
        auto value = serin::loadYaml("tests/data/sample2_users.yaml");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        auto users = obj["users"].asArray();
        CHECK(users.size() == 2);
        
        auto user1 = users[0].asObject();
        CHECK(user1["id"].asPrimitive() == 1.0);
        CHECK(user1["name"].asPrimitive() == "Alice");
        CHECK(user1["role"].asPrimitive() == "admin");
        
        auto user2 = users[1].asObject();
        CHECK(user2["id"].asPrimitive() == 2.0);
        CHECK(user2["name"].asPrimitive() == "Bob");
        CHECK(user2["role"].asPrimitive() == "user");
    }
}

TEST_CASE("Nested object serialization") {
    SUBCASE("JSON nested objects") {
        auto value = serin::loadJson("tests/data/sample3_nested.json");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        auto order = obj["order"].asObject();
        
        CHECK(order["id"].asPrimitive() == "ORD-12345");
        CHECK(order["status"].asPrimitive() == "completed");
        
        auto customer = order["customer"].asObject();
        CHECK(customer["name"].asPrimitive() == "John Doe");
        CHECK(customer["email"].asPrimitive() == "john@example.com");
        
        auto items = order["items"].asArray();
        CHECK(items.size() == 2);
        
        auto item1 = items[0].asObject();
        CHECK(item1["product"].asPrimitive() == "Book");
        CHECK(item1["quantity"].asPrimitive() == 2.0);
        CHECK(item1["price"].asPrimitive() == 15.0);
        
        auto item2 = items[1].asObject();
        CHECK(item2["product"].asPrimitive() == "Pen");
        CHECK(item2["quantity"].asPrimitive() == 5.0);
        CHECK(item2["price"].asPrimitive() == 2.5);
    }
    
    SUBCASE("TOON nested objects") {
        auto value = serin::loadToon("tests/data/sample3_nested.toon");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        auto order = obj["order"].asObject();
        
        CHECK(order["id"].asPrimitive() == "ORD-12345");
        CHECK(order["status"].asPrimitive() == "completed");
        
        auto customer = order["customer"].asObject();
        CHECK(customer["name"].asPrimitive() == "John Doe");
        CHECK(customer["email"].asPrimitive() == "john@example.com");
        
        auto items = order["items"].asArray();
        CHECK(items.size() == 2);
        
        auto item1 = items[0].asObject();
        CHECK(item1["product"].asPrimitive() == "Book");
        CHECK(item1["quantity"].asPrimitive() == 2.0);
        CHECK(item1["price"].asPrimitive() == 15.0);
        
        auto item2 = items[1].asObject();
        CHECK(item2["product"].asPrimitive() == "Pen");
        CHECK(item2["quantity"].asPrimitive() == 5.0);
        CHECK(item2["price"].asPrimitive() == 2.5);
    }
    
    SUBCASE("YAML nested objects") {
        auto value = serin::loadYaml("tests/data/sample3_nested.yaml");
        CHECK(value.isObject());
        
        auto obj = value.asObject();
        auto order = obj["order"].asObject();
        
        CHECK(order["id"].asPrimitive() == "ORD-12345");
        CHECK(order["status"].asPrimitive() == "completed");
        
        auto customer = order["customer"].asObject();
        CHECK(customer["name"].asPrimitive() == "John Doe");
        CHECK(customer["email"].asPrimitive() == "john@example.com");
        
        auto items = order["items"].asArray();
        CHECK(items.size() == 2);
        
        auto item1 = items[0].asObject();
        CHECK(item1["product"].asPrimitive() == "Book");
        CHECK(item1["quantity"].asPrimitive() == 2.0);
        CHECK(item1["price"].asPrimitive() == 15.0);
        
        auto item2 = items[1].asObject();
        CHECK(item2["product"].asPrimitive() == "Pen");
        CHECK(item2["quantity"].asPrimitive() == 5.0);
        CHECK(item2["price"].asPrimitive() == 2.5);
    }
}

TEST_CASE("Cross-format compatibility") {
    SUBCASE("JSON to TOON conversion") {
        auto jsonValue = serin::loadJson("tests/data/sample1_user.json");
        auto toonStr = serin::dumpsToon(jsonValue);
        
        // Parse TOON string back
        auto toonValue = serin::loadsToon(toonStr);
        CHECK(toonValue.isObject());
        
        auto obj = toonValue.asObject();
        CHECK(obj["name"].asPrimitive() == "Alice");
        CHECK(obj["age"].asPrimitive() == 30.0);
        CHECK(obj["active"].asPrimitive() == true);
    }
    
    SUBCASE("TOON to JSON conversion") {
        auto toonValue = serin::loadToon("tests/data/sample1_user.toon");
        auto jsonStr = serin::dumpsJson(toonValue);
        
        // Parse JSON string back
        auto jsonValue = serin::loadsJson(jsonStr);
        CHECK(jsonValue.isObject());
        
        auto obj = jsonValue.asObject();
        CHECK(obj["name"].asPrimitive() == "Alice");
        CHECK(obj["age"].asPrimitive() == 30.0);
        CHECK(obj["active"].asPrimitive() == true);
    }
    
    SUBCASE("YAML to JSON conversion") {
        auto yamlValue = serin::loadYaml("tests/data/sample1_user.yaml");
        auto jsonStr = serin::dumpsJson(yamlValue);
        
        // Parse JSON string back
        auto jsonValue = serin::loadsJson(jsonStr);
        CHECK(jsonValue.isObject());
        
        auto obj = jsonValue.asObject();
        CHECK(obj["name"].asPrimitive() == "Alice");
        CHECK(obj["age"].asPrimitive() == 30.0);
        CHECK(obj["active"].asPrimitive() == true);
    }
}
