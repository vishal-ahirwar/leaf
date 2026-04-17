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

-  **Fast project setup** - `leaf new` and you're ready to code
-  **Dependency management** - Add libraries with `leaf addpkg boost fmt spdlog`
-  **Build integration** - Automatic CMake generation and build orchestration
-  **Registry support** - Access to popular C++ libraries
-  **Parallel builds** - Multi-threaded compilation for faster builds
-  **Clean management** - Easy cleanup of build artifacts
-  **Detailed logging** - Beautiful, colored output with progress indicators

## Installation

### Install with scripts

#### Windows (`install.bat`)
```bat
powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-WebRequest -Uri https://raw.githubusercontent.com/vishal-ahirwar/leaf/master/install.bat -OutFile install.bat; .\install.bat"
```

#### macOS / Linux (`install.sh`)
```bash
curl -fsSL https://raw.githubusercontent.com/vishal-ahirwar/leaf/master/install.sh | bash
```

After installation, open a new terminal and verify:
```bash
leaf version
```
If you want to use mingw, this will setup mingw based toolchain on host os
```bash
leaf setup --mingw
```
If you want to use clang, this will install all the tools you need to compile your project
using clang, on windows it will install visual studio build tools too
```bash
leaf setup
```
### Build from source

### Prerequisites
- C++20 compatible compiler ( Clang 19++)
- CMake 3.20+
- Conan 2.0+ (for dependency resolution)

#### Using Leaf (Self-bootstrapping)
```bash
git clone --recursive https://github.com/vishal-ahirwar/leaf.git
cd leaf
leaf build  # Uses existing Leaf installation to build itself
```

#### Manual CMake Build
```bash
git clone --recursive https://github.com/vishal-ahirwar/leaf.git
cd leaf
python3 build.py
```

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

### Add dependencies
```bash
leaf addpkg fmt boost spdlog
```

### Build your project
```bash
leaf build          # Debug build
leaf build --release # Release build
leaf build --target <target>
```

### Run your project
```bash
leaf run
leaf run --release
leaf run --target <target>
leaf run --app <app>
```

### Test your project
```bash
leaf test
```



## Project Structure

Leaf follows a conventional project layout

## Advanced Usage

# Features you will see in upcoming release
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

#### Packages sharing
```bash
leaf publish
leaf publish --remote <remote>
```

#### Setup registry for private packages
```bash
leaf setup --registry
leaf serve --registry
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

- [x] **v0.1.0** - Basic project management and dependency resolution
- [-] **v0.2.0** - Package registry and publishing
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
