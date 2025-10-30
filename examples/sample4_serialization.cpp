#include "serin.h"
#include <iostream>

int main() {
    std::cout << "Serin Sample 4: Serialization Functions" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    // Create test data
    serin::Object data;
    data["name"] = serin::Value("Test User");
    data["age"] = serin::Value(30.0);
    data["active"] = serin::Value(true);
    
    serin::Array tags;
    tags.push_back(serin::Value("programming"));
    tags.push_back(serin::Value("c++"));
    tags.push_back(serin::Value("serialization"));
    data["tags"] = serin::Value(tags);
    
    serin::Value value(data);

    // Test JSON serialization
    std::cout << "JSON Serialization:" << std::endl;
    std::cout << "-------------------" << std::endl;
    std::string jsonStr = serin::dumpsJson(value);
    std::cout << "JSON string: " << jsonStr << std::endl;
    
    // Test TOON serialization
    std::cout << "\nTOON Serialization:" << std::endl;
    std::cout << "-------------------" << std::endl;
    std::string toonStr = serin::dumpsToon(value);
    std::cout << "TOON string:" << std::endl;
    std::cout << toonStr << std::endl;
    
    // Test file operations
    std::cout << "\nFile Operations:" << std::endl;
    std::cout << "----------------" << std::endl;
    
    try {
        // Save to JSON file
        serin::dumpJson(value, "test_output.json");
        std::cout << "Saved to test_output.json" << std::endl;
        
        // Save to TOON file
        serin::dumpToon(value, "test_output.toon");
        std::cout << "Saved to test_output.toon" << std::endl;
        
        // Load from JSON file
        serin::Value loadedJson = serin::loadJson("test_output.json");
        std::cout << "Loaded from JSON file" << std::endl;
        
        // Load from TOON file
        serin::Value loadedToon = serin::loadToon("test_output.toon");
        std::cout << "Loaded from TOON file" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
