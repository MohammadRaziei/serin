#include "serin.h"
#include <iostream>
#include <fstream>

int main() {
    std::cout << "Serin Sample 3: File I/O Operations" << std::endl;
    std::cout << "===================================" << std::endl << std::endl;

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

    try {
        // Save to TOON file
        serin::dumpToon(value, "sample_output.toon");
        std::cout << "Saved data to sample_output.toon" << std::endl;
        
        // Load from TOON file
        serin::Value loaded = serin::loadToon("sample_output.toon");
        std::cout << "Loaded data from sample_output.toon" << std::endl;
        
        // Display loaded data
        std::cout << "Loaded data:" << std::endl;
        std::cout << serin::dumpsToon(loaded) << std::endl;
        
        // Test JSON file operations
        serin::dumpJson(value, "sample_output.json");
        std::cout << "\nSaved data to sample_output.json" << std::endl;
        
        serin::Value jsonLoaded = serin::loadJson("sample_output.json");
        std::cout << "Loaded data from sample_output.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
