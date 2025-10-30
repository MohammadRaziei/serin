#include "toon.hpp"
#include <iostream>

using namespace toon;

int main() {
    std::cout << "TOON C++ - Tabular Format Example" << std::endl;
    std::cout << "=================================" << std::endl << std::endl;

    // Create tabular data (array of objects with identical structure)
    JsonArray employees;
    
    JsonObject emp1;
    emp1["id"] = JsonValue(101.0);
    emp1["name"] = JsonValue("Ali Rezaei");
    emp1["department"] = JsonValue("Engineering");
    emp1["salary"] = JsonValue(75000.0);
    emp1["active"] = JsonValue(true);
    employees.push_back(emp1);
    
    JsonObject emp2;
    emp2["id"] = JsonValue(102.0);
    emp2["name"] = JsonValue("Sara Mohammadi");
    emp2["department"] = JsonValue("Marketing");
    emp2["salary"] = JsonValue(65000.0);
    emp2["active"] = JsonValue(true);
    employees.push_back(emp2);
    
    JsonObject emp3;
    emp3["id"] = JsonValue(103.0);
    emp3["name"] = JsonValue("Reza Karimi");
    emp3["department"] = JsonValue("Sales");
    emp3["salary"] = JsonValue(70000.0);
    emp3["active"] = JsonValue(false);
    employees.push_back(emp3);
    
    JsonObject company;
    company["employees"] = JsonValue(employees);
    
    std::cout << "TOON Format (Tabular):" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << encode(JsonValue(company)) << std::endl;
    
    std::cout << std::endl << "Equivalent JSON would be much more verbose!" << std::endl;
    
    return 0;
}
