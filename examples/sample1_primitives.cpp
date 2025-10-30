#include "serin.h"
#include <iostream>

int main() {
    std::cout << "Serin Sample 1: Primitive Types" << std::endl;
    std::cout << "================================" << std::endl << std::endl;

    // Test all primitive types
    serin::Value str("Hello Serin");
    serin::Value num(42.5);
    serin::Value boolean(true);
    serin::Value nullValue(nullptr);
    
    std::cout << "String: " << serin::encode(str) << std::endl;
    std::cout << "Number: " << serin::encode(num) << std::endl;
    std::cout << "Boolean: " << serin::encode(boolean) << std::endl;
    std::cout << "Null: " << serin::encode(nullValue) << std::endl;
    
    return 0;
}
