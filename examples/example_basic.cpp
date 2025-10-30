#include "serin.h"
#include <iostream>

int main() {
    std::cout << "Serin C++ Library - Practical Examples" << std::endl;
    std::cout << "======================================" << std::endl << std::endl;

    // Example 1: Simple object with primitives
    std::cout << "Example 1: Simple Object" << std::endl;
    std::cout << "------------------------" << std::endl;
    {
        serin::Object user;
        user["id"] = serin::Value(123.0);
        user["name"] = serin::Value("Alice");
        user["active"] = serin::Value(true);
        user["score"] = serin::Value(95.5);
        
        std::cout << serin::encode(serin::Value(user)) << std::endl;
    }
    std::cout << std::endl;

    // Example 2: Nested objects
    std::cout << "Example 2: Nested Objects" << std::endl;
    std::cout << "-------------------------" << std::endl;
    {
        serin::Object address;
        address["street"] = serin::Value("123 Main St");
        address["city"] = serin::Value("Tehran");
        
        serin::Object person;
        person["name"] = serin::Value("Mohammad");
        person["age"] = serin::Value(30.0);
        person["address"] = serin::Value(address);
        
        std::cout << serin::encode(serin::Value(person)) << std::endl;
    }
    std::cout << std::endl;

    // Example 3: Primitive array
    std::cout << "Example 3: Primitive Array" << std::endl;
    std::cout << "--------------------------" << std::endl;
    {
        serin::Array tags;
        tags.push_back(serin::Value("programming"));
        tags.push_back(serin::Value("c++"));
        tags.push_back(serin::Value("serialization"));
        
        std::cout << serin::encode(serin::Value(tags)) << std::endl;
    }
    std::cout << std::endl;

    // Example 4: Tabular data (array of objects with same structure)
    std::cout << "Example 4: Tabular Data" << std::endl;
    std::cout << "-----------------------" << std::endl;
    {
        serin::Array users;
        
        serin::Object user1;
        user1["id"] = serin::Value(1.0);
        user1["name"] = serin::Value("Ali");
        user1["age"] = serin::Value(25.0);
        users.push_back(user1);
        
        serin::Object user2;
        user2["id"] = serin::Value(2.0);
        user2["name"] = serin::Value("Sara");
        user2["age"] = serin::Value(28.0);
        users.push_back(user2);
        
        serin::Object user3;
        user3["id"] = serin::Value(3.0);
        user3["name"] = serin::Value("Reza");
        user3["age"] = serin::Value(32.0);
        users.push_back(user3);
        
        serin::Object data;
        data["users"] = serin::Value(users);
        
        std::cout << serin::encode(serin::Value(data)) << std::endl;
    }
    std::cout << std::endl;

    // Example 5: Mixed data with different delimiters
    std::cout << "Example 5: Tab Delimited Data" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    {
        serin::Array products;
        
        serin::Object product1;
        product1["sku"] = serin::Value("P-001");
        product1["name"] = serin::Value("Laptop");
        product1["price"] = serin::Value(1200.0);
        products.push_back(product1);
        
        serin::Object product2;
        product2["sku"] = serin::Value("P-002");
        product2["name"] = serin::Value("Mouse");
        product2["price"] = serin::Value(25.5);
        products.push_back(product2);
        
        serin::Object catalog;
        catalog["products"] = serin::Value(products);
        
        serin::EncodeOptions options;
        options.delimiter = serin::Delimiter::Tab;
        std::cout << serin::encode(serin::Value(catalog), options) << std::endl;
    }
    std::cout << std::endl;

    // Example 6: Complex nested structure
    std::cout << "Example 6: Complex Nested Structure" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    {
        serin::Object order;
        order["orderId"] = serin::Value("ORD-12345");
        order["status"] = serin::Value("completed");
        
        serin::Array items;
        
        serin::Object item1;
        item1["product"] = serin::Value("Book");
        item1["quantity"] = serin::Value(2.0);
        item1["price"] = serin::Value(15.0);
        items.push_back(item1);
        
        serin::Object item2;
        item2["product"] = serin::Value("Pen");
        item2["quantity"] = serin::Value(5.0);
        item2["price"] = serin::Value(2.5);
        items.push_back(item2);
        
        order["items"] = serin::Value(items);
        
        serin::Object customer;
        customer["name"] = serin::Value("John Doe");
        customer["email"] = serin::Value("john@example.com");
        order["customer"] = serin::Value(customer);
        
        std::cout << serin::encode(serin::Value(order)) << std::endl;
    }
    std::cout << std::endl;

    std::cout << "All examples completed successfully!" << std::endl;
    return 0;
}
