#include "toon.hpp"
#include <iostream>

using namespace toon;

int main() {
    std::cout << "TOON Sample 2: Objects and Nesting" << std::endl;
    std::cout << "==================================" << std::endl << std::endl;

    // Create nested object structure
    JsonObject config;
    config["appName"] = JsonValue("MyApp");
    config["version"] = JsonValue("1.0.0");
    config["debug"] = JsonValue(true);
    
    JsonObject database;
    database["host"] = JsonValue("localhost");
    database["port"] = JsonValue(5432.0);
    database["name"] = JsonValue("mydb");
    
    config["database"] = JsonValue(database);
    
    JsonObject cache;
    cache["enabled"] = JsonValue(true);
    cache["ttl"] = JsonValue(3600.0);
    
    config["cache"] = JsonValue(cache);
    
    std::cout << "Configuration Object:" << std::endl;
    std::cout << encode(JsonValue(config)) << std::endl;
    
    return 0;
}
