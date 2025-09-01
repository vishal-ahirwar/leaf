# -----------------------------------------------------------------------------
# MONOREPO-AWARE CONAN RECIPE
#
# This recipe can:
#  - Build everything for local development (`conan install .`)
#  - Build and package ONLY the library (`conan create . -o build_app=False`)
#
# Date: 2025-09-01
# -----------------------------------------------------------------------------
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout

# (ConsumerRole and PublisherRole mixins can remain the same as the previous template)
class ConsumerRole:
    def requirements(self):
        self.requires("fmt/11.2.0")
        self.requires("reproc/14.2.5")
        self.requires("cpr/1.12.0")
        
class PublisherRole:
    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        # --- THE KEY MODIFICATION ---
        # Pass the value of our Conan option to a CMake variable.
        tc.variables["BUILD_APPLICATION"] = False
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        # This package only exposes information about the library.
        self.cpp_info.libs = ["my_lib"]

# --- Final ConanFile ---
class MyProjectConan(ConsumerRole, PublisherRole, ConanFile):
    # --- I. CORE METADATA ---
    name = "my_lib" # The package name should be the library's name
    version = "0.1.0"
    # ... (license, author, url, etc. remain the same)

    # --- II. CONFIGURATION ---
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True
    }

    # --- III. SOURCE MANAGEMENT ---
    # Make sure to export ALL necessary source code.
    exports_sources = "CMakeLists.txt", "app/*", "libs/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")