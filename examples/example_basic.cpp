#include "toon.hpp"
#include <iostream>

using namespace toon;

int main() {
    std::cout << "TOON C++ Library - Practical Examples" << std::endl;
    std::cout << "=====================================" << std::endl << std::endl;

    // Example 1: Simple object with primitives
    std::cout << "Example 1: Simple Object" << std::endl;
    std::cout << "------------------------" << std::endl;
    {
        JsonObject user;
        user["id"] = JsonValue(123.0);
        user["name"] = JsonValue("Alice");
        user["active"] = JsonValue(true);
        user["score"] = JsonValue(95.5);
        
        std::cout << encode(JsonValue(user)) << std::endl;
    }
    std::cout << std::endl;

    // Example 2: Nested objects
    std::cout << "Example 2: Nested Objects" << std::endl;
    std::cout << "-------------------------" << std::endl;
    {
        JsonObject address;
        address["street"] = JsonValue("123 Main St");
        address["city"] = JsonValue("Tehran");
        
        JsonObject person;
        person["name"] = JsonValue("Mohammad");
        person["age"] = JsonValue(30.0);
        person["address"] = JsonValue(address);
        
        std::cout << encode(JsonValue(person)) << std::endl;
    }
    std::cout << std::endl;

    // Example 3: Primitive array
    std::cout << "Example 3: Primitive Array" << std::endl;
    std::cout << "--------------------------" << std::endl;
    {
        JsonArray tags;
        tags.push_back(JsonValue("programming"));
        tags.push_back(JsonValue("c++"));
        tags.push_back(JsonValue("serialization"));
        
        std::cout << encode(JsonValue(tags)) << std::endl;
    }
    std::cout << std::endl;

    // Example 4: Tabular data (array of objects with same structure)
    std::cout << "Example 4: Tabular Data" << std::endl;
    std::cout << "-----------------------" << std::endl;
    {
        JsonArray users;
        
        JsonObject user1;
        user1["id"] = JsonValue(1.0);
        user1["name"] = JsonValue("Ali");
        user1["age"] = JsonValue(25.0);
        users.push_back(user1);
        
        JsonObject user2;
        user2["id"] = JsonValue(2.0);
        user2["name"] = JsonValue("Sara");
        user2["age"] = JsonValue(28.0);
        users.push_back(user2);
        
        JsonObject user3;
        user3["id"] = JsonValue(3.0);
        user3["name"] = JsonValue("Reza");
        user3["age"] = JsonValue(32.0);
        users.push_back(user3);
        
        JsonObject data;
        data["users"] = JsonValue(users);
        
        std::cout << encode(JsonValue(data)) << std::endl;
    }
    std::cout << std::endl;

    // Example 5: Mixed data with different delimiters
    std::cout << "Example 5: Tab Delimited Data" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    {
        JsonArray products;
        
        JsonObject product1;
        product1["sku"] = JsonValue("P-001");
        product1["name"] = JsonValue("Laptop");
        product1["price"] = JsonValue(1200.0);
        products.push_back(product1);
        
        JsonObject product2;
        product2["sku"] = JsonValue("P-002");
        product2["name"] = JsonValue("Mouse");
        product2["price"] = JsonValue(25.5);
        products.push_back(product2);
        
        JsonObject catalog;
        catalog["products"] = JsonValue(products);
        
        EncodeOptions options;
        options.delimiter = Delimiter::Tab;
        std::cout << encode(JsonValue(catalog), options) << std::endl;
    }
    std::cout << std::endl;

    // Example 6: Complex nested structure
    std::cout << "Example 6: Complex Nested Structure" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    {
        JsonObject order;
        order["orderId"] = JsonValue("ORD-12345");
        order["status"] = JsonValue("completed");
        
        JsonArray items;
        
        JsonObject item1;
        item1["product"] = JsonValue("Book");
        item1["quantity"] = JsonValue(2.0);
        item1["price"] = JsonValue(15.0);
        items.push_back(item1);
        
        JsonObject item2;
        item2["product"] = JsonValue("Pen");
        item2["quantity"] = JsonValue(5.0);
        item2["price"] = JsonValue(2.5);
        items.push_back(item2);
        
        order["items"] = JsonValue(items);
        
        JsonObject customer;
        customer["name"] = JsonValue("John Doe");
        customer["email"] = JsonValue("john@example.com");
        order["customer"] = JsonValue(customer);
        
        std::cout << encode(JsonValue(order)) << std::endl;
    }
    std::cout << std::endl;

    std::cout << "All examples completed successfully!" << std::endl;
    return 0;
}
