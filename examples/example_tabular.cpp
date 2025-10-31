#include "serin.h"
#include <iostream>

int main() {
    std::cout << "Serin Example: Tabular Data Format" << std::endl;
    std::cout << "==================================" << std::endl << std::endl;

    // Create employee data
    serin::Array employees;
    
    // Employee 1
    serin::Object emp1;
    emp1["id"] = serin::Value(101.0);
    emp1["name"] = serin::Value("Ali Rezaei");
    emp1["department"] = serin::Value("Engineering");
    emp1["salary"] = serin::Value(75000.0);
    emp1["active"] = serin::Value(true);
    employees.push_back(emp1);
    
    // Employee 2
    serin::Object emp2;
    emp2["id"] = serin::Value(102.0);
    emp2["name"] = serin::Value("Sara Mohammadi");
    emp2["department"] = serin::Value("Marketing");
    emp2["salary"] = serin::Value(65000.0);
    emp2["active"] = serin::Value(true);
    employees.push_back(emp2);
    
    // Employee 3 (inactive)
    serin::Object emp3;
    emp3["id"] = serin::Value(103.0);
    emp3["name"] = serin::Value("Reza Karimi");
    emp3["department"] = serin::Value("Sales");
    emp3["salary"] = serin::Value(70000.0);
    emp3["active"] = serin::Value(false);
    employees.push_back(emp3);
    
    serin::Object data;
    data["employees"] = serin::Value(employees);
    
    std::cout << "TOON Format Output:" << std::endl;
    std::cout << "-------------------" << std::endl;
    std::cout << serin::dumpsToon(serin::Value(data)) << std::endl;
    
    return 0;
}
