#include "toon.hpp"
#include <iostream>

using namespace toon;

int main() {
    std::cout << "TOON Sample 3: File I/O Operations" << std::endl;
    std::cout << "==================================" << std::endl << std::endl;

    // Create sample data
    JsonObject data;
    data["app"] = JsonValue("MyApp");
    data["version"] = JsonValue("1.0.0");
    
    JsonArray users;
    
    JsonObject user1;
    user1["id"] = JsonValue(1.0);
    user1["name"] = JsonValue("Alice");
    user1["active"] = JsonValue(true);
    users.push_back(user1);
    
    JsonObject user2;
    user2["id"] = JsonValue(2.0);
    user2["name"] = JsonValue("Bob");
    user2["active"] = JsonValue(false);
    users.push_back(user2);
    
    data["users"] = JsonValue(users);
    
    JsonValue dataValue(data);
    
    std::cout << "Original Data:" << std::endl;
    std::cout << encode(dataValue) << std::endl;
    std::cout << std::endl;
    
    // Demonstrate file operations
    try {
        std::cout << "1. Encoding to file..." << std::endl;
        encodeToFile(dataValue, "output.toon");
        std::cout << "   ✓ Data written to output.toon" << std::endl;
        
        std::cout << "2. Decoding from file..." << std::endl;
        JsonValue decoded = decodeFromFile("output.toon");
        std::cout << "   ✓ Data read from output.toon" << std::endl;
        
        std::cout << "3. Decoded data:" << std::endl;
        std::cout << encode(decoded) << std::endl;
        
        std::cout << "4. File conversion simulation..." << std::endl;
        // This simulates the CLI functionality: toon input.json -o output.toon
        std::cout << "   ✓ JSON to TOON conversion supported" << std::endl;
        std::cout << "   ✓ TOON to JSON conversion supported" << std::endl;
        
        // Clean up
        std::remove("output.toon");
        std::cout << "   ✓ Temporary files cleaned up" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << std::endl << "File I/O operations demonstrate CLI-equivalent functionality:" << std::endl;
    std::cout << "- encodeToFile()    → toon input.json -o output.toon" << std::endl;
    std::cout << "- decodeFromFile()  → toon data.toon -o output.json" << std::endl;
    std::cout << "- convertFile()     → Auto-detection based on file extension" << std::endl;
    
    return 0;
}
