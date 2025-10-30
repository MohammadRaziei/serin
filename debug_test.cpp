#include "includes/toon.hpp"
#include <iostream>

int main() {
    try {
        // Test with a simple JSON string
        std::string json = R"({"users": [{"id": 1, "name": "Alice", "role": "admin"}, {"id": 2, "name": "Bob", "role": "user"}]})";
        
        std::cout << "Testing JSON parsing..." << std::endl;
        toon::Value parsed = toon::parseJson(json);
        std::cout << "Parsed successfully!" << std::endl;
        
        std::cout << "Testing encoding..." << std::endl;
        std::string toonOutput = toon::encode(parsed);
        std::cout << "TOON output:" << std::endl;
        std::cout << toonOutput << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
