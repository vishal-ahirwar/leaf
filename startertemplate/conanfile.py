from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
    
class %APPNAME%Conan(ConanFile):
    name = "%APPNAME%" # The package name should be the library's name
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
    exports_sources = "CMakeLists.txt", "app/*", "libs/*","cmake/*"
    
    def requirements(self):
        #self.requires("Libname/version")
        if self.options.build_app:  # Only for the app
            pass
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
        # This package only exposes information about the library.
        self.cpp_info.libs = ["%LIBNAME%"] #Only if you have single lib if you component then
        # # Define the "yourlib" library component
        # self.cpp_info.components["yourlib"].libs = ["yourlib"]
        # self.cpp_info.components["yourlib"].requires = ["fmt::fmt"] # Example if generator depends on fmt