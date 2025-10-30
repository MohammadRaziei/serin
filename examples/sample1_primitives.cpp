#include "toon.hpp"
#include <iostream>

using namespace toon;

int main() {
    std::cout << "TOON Sample 1: Primitive Types" << std::endl;
    std::cout << "==============================" << std::endl << std::endl;

    // Test all primitive types
    JsonValue str("Hello TOON");
    JsonValue num(42.5);
    JsonValue boolean(true);
    JsonValue nullValue(nullptr);
    
    std::cout << "String: " << encode(str) << std::endl;
    std::cout << "Number: " << encode(num) << std::endl;
    std::cout << "Boolean: " << encode(boolean) << std::endl;
    std::cout << "Null: " << encode(nullValue) << std::endl;
    
    return 0;
}
