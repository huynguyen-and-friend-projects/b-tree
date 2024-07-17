from conan import ConanFile

# this file is for Windows workflow because the thing keeps failing

class BTreeRecipy(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("gtest/1.14.0")
