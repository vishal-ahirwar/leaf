from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
    
class LeafConan(ConanFile):
    name = "leaf" # The package name should be the library's name
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_app": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "build_app": False
    }
    # Make sure to export ALL necessary source code.
    exports_sources = "CMakeLists.txt", "libs/*","cmake/*"
    
    def requirements(self):
        self.requires("fmt/11.2.0")
        self.requires("reproc/14.2.5")
        self.requires("cpr/1.12.0")
        self.requires("nlohmann_json/3.12.0")
        self.requires("pystring/1.1.4")
        self.requires("platformfolders/4.3.0")
        if self.options.build_app:  # Only for the app
            self.requires("gtest/1.17.0")

        else: # Only for the libs
            pass
    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        #NOTE: This is if you want to publish apps with libs too
        tc.variables["BUILD_APPLICATION"] = self.options.build_app
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        # Define the "utils" library component
        self.cpp_info.components["utils"].libs = ["utils"]
        self.cpp_info.components["utils"].requires=["fmt::fmt"]
        # Define the "utils" library component
        self.cpp_info.components["easyproc"].libs = ["easyproc"]
        self.cpp_info.components["easyproc"].requires=["reproc::reproc"]
        # # CRITICAL: Declare ALL public dependencies for the 'utils' component.
        self.cpp_info.components["commands"].libs = ["commands"]
        self.cpp_info.components["commands"].requires=["fmt::fmt"]
        # # Define the "downloader" library component
        self.cpp_info.components["downloader"].libs = ["downloader"]
        self.cpp_info.components["downloader"].requires = ["utils","cpr::cpr","nlohmann_json::nlohmann_json"] # Example if downloader depends on utils

        # # Define the "generator" library component
        # self.cpp_info.components["generator"].libs = ["generator"]
        # self.cpp_info.components["generator"].requires = ["utils"] # Example if generator depends on utils