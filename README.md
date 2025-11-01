# Serin - Serialized In

<div align="center">
  <div style="display: flex; align-items: center; justify-content: center; gap: 40px;">
    <div style="flex: 0 0 auto;">
      <img src="docs/images/serin.svg" alt="Serin Logo" width="130" >
    </div>
    <div style="flex: 1;">
      <p><strong>Serin</strong> (acronym for Serialized In) is a C++ and Python serialization library that supports multiple formats like JSON, YAML, and TOON, and can convert data from any format to another.</p>
      <p>This project is written using the C++17 standard and is highly efficient and fast.</p>
    </div>
  </div>
</div>

## 🚀 Features

- **Multiple Format Support**: JSON, YAML, and TOON
- **Bidirectional Conversion**: Convert data between all supported formats
- **High Performance**: Optimized with C++17 for fast performance
- **Python Friendly**: Python interfaces for easy use
- **Powerful CLI**: Command-line tool for file conversion
- **TOON Integration**: Full TOON format support for token efficiency in LLM

## 📦 Installation

### C++

```bash
# Clone the repository
git clone https://github.com/MohammadRaziei/serin.git
cd serin

# Build the project
mkdir build && cd build
cmake ..
make

# Install
sudo make install
```

### Python

```python
# Install from source
pip install .

# Or use the Python bindings directly
import serin
```

## 🎯 Quick Start

### Using C++

```cpp
#include "serin.h"
#include <iostream>

int main() {
    // Create sample data
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

    // Serialize to JSON
    std::string jsonStr = serin::dumpsJson(value);
    std::cout << "JSON: " << jsonStr << std::endl;
    
    // Serialize to TOON
    std::string toonStr = serin::dumpsToon(value);
    std::cout << "TOON:" << std::endl;
    std::cout << toonStr << std::endl;
    
    return 0;
}
```

### Using Python

```python
import serin

# Create sample data
data = {
    "name": "Test User",
    "age": 30,
    "active": True,
    "tags": ["programming", "c++", "serialization"]
}

# Serialize to JSON
json_str = serin.dumps_json(data)
print(f"JSON: {json_str}")

# Serialize to TOON
toon_str = serin.dumps_toon(data)
print(f"TOON:\n{toon_str}")

# Save to file
serin.dump_json(data, "output.json")
serin.dump_toon(data, "output.toon")

# Load from file
loaded_data = serin.load_json("output.json")
print(f"Loaded data: {loaded_data}")
```

## 🔧 Command Line Interface (CLI)

Serin includes a powerful command-line tool for file conversion:

```bash
# Show help and available options
serin --help

# Show the current Serin version
serin --version

# Convert JSON to TOON and write to a file (format inferred from extension)
serin input.json -o output.toon

# Output a conversion directly to the terminal (defaults to TOON)
serin input.json

# Select an explicit output format when streaming to stdout
serin input.json -t yaml

# Control indentation for structured formats
serin data.yaml -t json -i 4
```

## 📊 TOON Format

TOON (Token-Oriented Object Notation) is a compact, human-readable format designed for transferring structured data to Large Language Models (LLMs) with significantly lower token consumption.

### TOON Example

```json
// JSON
{
  "users": [
    { "id": 1, "name": "Alice", "role": "admin" },
    { "id": 2, "name": "Bob", "role": "user" }
  ]
}
```

```
// TOON (42.3% fewer tokens)
users[2]{id,name,role}:
  1,Alice,admin
  2,Bob,user
```

### TOON Advantages

- **💸 Token Efficiency**: Typically 30-60% fewer tokens than JSON
- **🤿 LLM Guidance**: Explicit lengths and field lists help models
- **🍱 Minimal Syntax**: Eliminates repetitive punctuation
- **📐 Indentation-based Structure**: Replaces braces with whitespace
- **🧺 Tabular Arrays**: Declare keys once, stream rows

## 🧪 Examples

The project includes multiple examples for quick start:

- `examples/example_basic.cpp` - Basic examples
- `examples/sample4_serialization.cpp` - Serialization functions
- `examples/example_tabular.cpp` - Tabular data

## 📚 API

### Main Functions

- `loadJson(filename)` / `loadsJson(string)` - Load JSON
- `dumpJson(value, filename)` / `dumpsJson(value)` - Save JSON
- `loadToon(filename)` / `loadsToon(string)` - Load TOON
- `dumpToon(value, filename)` / `dumpsToon(value)` - Save TOON
- `loadYaml(filename)` / `loadsYaml(string)` - Load YAML
- `dumpYaml(value, filename)` / `dumpsYaml(value)` - Save YAML

### Data Structures

- `serin::Value` - Main data type
- `serin::Object` - Object (dictionary)
- `serin::Array` - Array
- `serin::Primitive` - Primitive values (string, number, boolean, null)
- `serin::ToonOptions` - Configure TOON serialization (indentation, delimiter, strict mode)

### TOON Configuration

```cpp
serin::ToonOptions options;
options.setIndent(4).setDelimiter(serin::Delimiter::Pipe);

auto toon = serin::dumpsToon(value, options);
```

To confirm that Serin's TOON output matches the reference implementation, run the C++ tests with
`npx` available. The `TOON output matches @byjohann/toon when available` test will invoke
`npx @byjohann/toon` to compare the serialized output whenever the package can be downloaded.

## 🧪 Tests

```bash
# Run tests
cd build
make test

# Or directly
./test_serin
```

## 🤝 Contribution

Contributions are always welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Open a pull request

## 📄 License

This project is released under the MIT License.

## 👤 Developer

- **Mohammad Raziei** - [GitHub](https://github.com/MohammadRaziei)

## 🙌 Acknowledgments

- [TOON](https://github.com/byjohann/toon) project for inspiration
- Open source community for tools and libraries

---

<p align="center">
  Made with ❤️ in Iran
</p>
