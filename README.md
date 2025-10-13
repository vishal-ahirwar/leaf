# 🍃 Leaf

A modern, fast, and intuitive package manager for C++ - inspired by Cargo for Rust.

<img width="988" height="547" alt="Screenshot 2025-10-13 051016" src="https://github.com/user-attachments/assets/603bf77f-8158-4163-b238-eb2ea29982f3" />

## Overview

Leaf aims to bring the simplicity and power of Cargo to the C++ ecosystem. Just like how Cargo revolutionized Rust development, Leaf streamlines C++ project management with:

- **Simple project initialization** - Get started with a single command
- **Dependency management** - Add, remove, and update dependencies effortlessly  
- **Build system integration** - Works seamlessly with CMake and other build systems
- **Package registry** - Discover and share C++ libraries
- **Cross-platform support** - Windows, Linux, and macOS

## Features

- 🚀 **Fast project setup** - `leaf new my-project` and you're ready to code
- 📦 **Dependency management** - Add libraries with `leaf add boost fmt spdlog`
- 🔧 **Build integration** - Automatic CMake generation and build orchestration
- 🌐 **Registry support** - Access to popular C++ libraries
- ⚡ **Parallel builds** - Multi-threaded compilation for faster builds
- 🧹 **Clean management** - Easy cleanup of build artifacts
- 📊 **Detailed logging** - Beautiful, colored output with progress indicators

## Installation

### From Source

#### Using Leaf (Self-bootstrapping)
```bash
git clone https://github.com/vishal-ahirwar/leaf.git
cd leaf
leaf build  # Uses existing Leaf installation to build itself
```

#### Manual CMake Build
```bash
git clone https://github.com/vishal-ahirwar/leaf.git
cd leaf
conan install . -of=.install
cmake -B .build
cmake --build .build
```

### Prerequisites
- C++20 compatible compiler ( Clang 19++)
- CMake 3.20+
- Conan 2.0+ (for dependency resolution)

### Development Tools Generated
Leaf automatically generates development configuration files:
- `.clang-format` - Code formatting rules
- `compile_commands.json` - For clangd LSP support
- `.clangd` - Language server configuration

## Quick Start

### Create a new project
```bash
leaf new hello-world
cd hello-world
```

This creates a project structure like:
```
leaf/
├── conanfile.py          # Root conanfile
├── CMakeLists.txt        # Root CMakeLists to orchestrate the entire build.
│
├── apps/                 # Contains all final executable applications.
│   └── leaf/             # An example application named 'leaf'.
│       ├── src/
│       │   └── main.cpp
│       └── CMakeLists.txt  # Build script for the 'leaf' application.
│
├── libs/                 # Contains all shared/static libraries.
│   ├── downloader/       # A library for downloading files.
│   │   ├── src/
│   │   └── CMakeLists.txt
│   ├── generator/        # A library for generating content.
│   │   ├── src/
│   │   └── CMakeLists.txt
│   └── utils/            # A general-purpose utility library.
│       ├── src/
│       └── CMakeLists.txt
│
├── .vscode/              # VS Code editor settings (optional).
├── build/                # Build output directory (auto-generated).
├── .install/             # Conan dependencies (auto-generated).
├── cmake/                # Custom CMake modules (optional).
└── .gitignore            # Git ignore rules.
```

### Add dependencies
```bash
leaf add fmt boost spdlog
```

### Build your project
```bash
leaf build          # Debug build
leaf build --release # Release build
```

### Run your project
```bash
leaf run
leaf run --release
```

### Test your project
```bash
leaf test
```

## Commands

| Command | Description |
|---------|-------------|
| `leaf new <name>` | Create a new C++ project |
| `leaf init` | Initialize Leaf in an existing project |
| `leaf add <package>` | Add a dependency |
| `leaf remove <package>` | Remove a dependency |
| `leaf build` | Build the project |
| `leaf run` | Build and run the project |
| `leaf test` | Run tests |
| `leaf clean` | Clean build artifacts |
| `leaf update` | Update dependencies |
| `leaf search <query>` | Search for packages |
| `leaf info <package>` | Show package information |

## Configuration

### conanfile.py
```python
from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain, CMake
from conan.tools.files import copy

class MyAwesomeProjectConan(ConanFile):
    name = "my-awesome-project"
    version = "1.0.0"
    description = "An awesome C++ project"
    author = "Your Name <you@example.com>"
    license = "MIT"
    
    # Package configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_networking": [True, False],
        "with_experimental": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_networking": True,
        "with_experimental": False
    }
    
    # Sources are located in the same place as this recipe
    exports_sources = "CMakeLists.txt", "src/*", "include/*"
    
    def requirements(self):
        self.requires("fmt/10.2.1")
        self.requires("boost/1.84.0")
        
        if self.options.with_networking:
            self.requires("openssl/3.1.4")
    
    def build_requirements(self):
        self.test_requires("catch2/3.4.0")
        self.test_requires("benchmark/1.8.3")
    
    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_CXX_STANDARD"] = "20"
        tc.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
    
    def package(self):
        cmake = CMake(self)
        cmake.install()
```

## Project Structure

Leaf follows a conventional project layout:

```
my-project/
├── conanfile.py           # Conan package definition
├── CMakeLists.txt         # CMake build configuration  
├── .leafconfig            # Leaf-specific settings
├── README.md
├── .gitignore
├── my-project/            # Root project
│   └── src/
│       └── main.cpp      # Application entry point
├── sub-project1/         # Sub-project (library/executable)
│   ├── src/
│   │   └── source.cpp
│   └── include/
│       └── sub-project1.h
├── sub-project2/         # Another sub-project
│   ├── src/
│   │   └── source.cpp
│   └── include/
│       └── sub-project2.h
├── tests/                # Unit tests
│   └── test_main.cpp
├── examples/             # Example code
│   └── basic_usage.cpp
├── docs/                 # Documentation
├── install/              # Conan dependencies (auto-generated)
├── config/               # Project configuration files
└── build/                # Build output (auto-generated)
    ├── debug/
    └── release/
```

## Advanced Usage

### Custom Build Scripts
```bash
# Pre-build script
leaf build --script prebuild.py

# Post-build hook
leaf build --post-script "cpack -G DEB"
```

### Cross-compilation
```bash
# Android cross-compilation
leaf build --target android-arm64
leaf build --target android-x86_64

# Web/WASM compilation 
leaf build --target wasm32-emscripten
leaf build --target wasm32-wasi

# Other targets
leaf build --target x86_64-linux-gnu
leaf build --target aarch64-apple-darwin
leaf build --target x86_64-pc-windows-msvc
```

#### Android Cross-compilation Setup
```bash
# Build for Android
leaf build --target android-arm64
```

#### Web/WASM Compilation Setup  
```bash
# Build for Web
leaf build --target wasm32-emscripten

# The output will be .wasm + .js files ready for web deployment
```

## Integration with IDEs

### CLion & Vscode
Leaf automatically generates CMake files and `compile_commands.json` that CLion can import directly. The generated `.clangd` configuration provides optimal IntelliSense support.

### Vim/Neovim
With generated development files:
- `compile_commands.json` - For LSP completion via clangd
- `.clangd` - Configured for optimal C++ support
- `.clang-format` - Consistent code formatting


## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Setup

#### Quick Start (Self-bootstrapping)
```bash
git clone https://github.com/vishal-ahirwar/leaf.git
cd leaf
leaf build   # Development build with debug info
leaf test          # Run all tests
```

#### Using CMake Presets
```bash
git clone https://github.com/vishal-ahirwar/leaf.git
cd leaf
conan install . -of=.install
# Development build with all tools
cmake --preset=clang-posix
cmake --build --preset=clang-posix
```

#### Development Tools Integration
The build system automatically configures:

**.clang-format**
```yaml
BasedOnStyle: Google
IndentWidth: 2
ColumnLimit: 100
SortIncludes: true
```

**.clangd** 
```yaml
CompileFlags:
  Add: [-std=c++20, -Wall, -Wextra]
  Remove: [-W*, -std=*]
Index:
  Background: Build
```

**compile_commands.json**
Generated automatically for perfect LSP integration with any editor.

## Roadmap

- [ ] **v0.1.0** - Basic project management and dependency resolution
- [ ] **v0.2.0** - Package registry and publishing
- [ ] **v0.3.0** - Workspace support and multi-project management
- [ ] **v0.4.0** - IDE integrations and tooling
- [ ] **v0.5.0** - Cross-compilation and target management
- [ ] **v1.0.0** - Stable API and production ready

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by [Cargo](https://doc.rust-lang.org/cargo/) for Rust
- Built on top of [Conan](https://conan.io/) for dependency resolution  
- Uses [CMake](https://cmake.org/) for build system generation

---

**Made with ❤️ for the C++ community**

*"Just like a leaf makes the tree beautiful, Leaf makes C++ development beautiful."*
