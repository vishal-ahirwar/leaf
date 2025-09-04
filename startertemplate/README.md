# %APPNAME%

[![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)](https://www.linux.org/) [![macOS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=macos&logoColor=F0F0F0)](https://www.apple.com/macos) [![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)](https://www.microsoft.com/windows) [![Web](https://img.shields.io/badge/Web-3498DB?style=for-the-badge&logo=WebAssembly&logoColor=white)](https://webassembly.org/)

A brief description of what `%APPNAME%` does and who it's for. This project contains the application `%APPNAME%` and the library `%LIBNAME%`.

---

## 🚀 Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

Before you begin, ensure you have the following tools installed on your system:

-   **C++ Compiler**: [Clang](https://clang.llvm.org/) version 20 or newer.
-   **Project Manager**: [Leaf](https://github.com/vishal-ahirwar/leaf)

### Building the Project

Follow these steps to build and run the project locally.

1.  **Clone the repository:**
    ```sh
    git clone <your-repository-url>
    cd %APPNAME%
    ```

2.  **Compile the code:**
    Run leaf to build the project.
    ```sh
    leaf compile
    ```

---

## ▶️ Usage

After a successful build, you can run the application using leaf.

To run the application, execute the following command from the project root:
```sh
leaf run %APPNAME%
```

---

## 🛠️ Toolchain

This project is configured with a specific set of modern C++ tools:

-   **Compiler**: `Clang C++20`
-   **Project Manager**: `Leaf` for template generation and project management, which handles dependencies (via Conan) and building (via CMake and Ninja) for you.

### Tested Platforms

This Project has been tested and is expected to work on the following platforms:
-   **Linux** (Ubuntu, Arch, etc.)
-   **macOS**
-   **Windows** (with MSVC or ClangCL)
-   **Web** (via Emscripten/WebAssembly)

---

## 🤝 Contributing

Contributions are what make the open-source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

Please refer to the project's issue tracker and pull request guidelines.

## 📄 License

This project is licensed under the MIT License - see the `LICENSE.md` file for details.
