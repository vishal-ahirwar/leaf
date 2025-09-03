from conan import ConanFile

class %PROJECT_NAME%(ConanFile):
    name = "%PROJECT_NAME%"
    version = "0.1.0"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("fmt/10.2.1")

    def layout(self):
        self.folders.build = "build"
        self.folders.generators = "build/generators"
