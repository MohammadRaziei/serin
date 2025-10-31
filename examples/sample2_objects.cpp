#include "serin.h"
#include <iostream>

int main() {
    std::cout << "Serin Sample 2: Object Types" << std::endl;
    std::cout << "============================" << std::endl << std::endl;

    // Create a nested object structure
    serin::Object address;
    address["street"] = serin::Value("123 Main St");
    address["city"] = serin::Value("Tehran");
    address["country"] = serin::Value("Iran");
    
    serin::Object person;
    person["name"] = serin::Value("Mohammad");
    person["age"] = serin::Value(30.0);
    person["address"] = serin::Value(address);
    
    serin::Array hobbies;
    hobbies.push_back(serin::Value("programming"));
    hobbies.push_back(serin::Value("reading"));
    hobbies.push_back(serin::Value("hiking"));
    person["hobbies"] = serin::Value(hobbies);
    
    std::cout << "Nested Object:" << std::endl;
    std::cout << serin::dumpsToon(serin::Value(person)) << std::endl;
    
    return 0;
}
