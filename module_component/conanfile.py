from conans import ConanFile, CMake, tools


class UbitrackCoreConan(ConanFile):
    name = "ubitrack_module_component"
    version = "1.3.0"

    description = "Ubitrack Module component example"
    url = "URL to your component git"
    license = "Your Licence"

    short_paths = True
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    options = { 
        "workspaceBuild" : [True, False],
    }

    default_options = {
        "ubitrack_core:shared":True,
        "ubitrack_dataflow:shared":True,
		"ubitrack_vision:shared":True,
        "workspaceBuild" : False,
        }

    # all sources are deployed with the package
    exports_sources = "doc/*", "src/*","include/*", "CMakeLists.txt"

    def configure(self):
        if self.settings.os == "Linux":
            self.options["opencv"].with_gtk = True

    def requirements(self):
        userChannel = "ubitrack/stable"
        if self.options.workspaceBuild:
            userChannel = "local/dev"

        self.requires("ubitrack_core/%s@%s" % (self.version, userChannel))
        self.requires("ubitrack_vision/%s@%s" % (self.version, userChannel))
        self.requires("ubitrack_dataflow/%s@%s" % (self.version, userChannel))

    # def imports(self):
    #     self.copy(pattern="*.dll", dst="bin", src="bin") # From bin to bin
    #     self.copy(pattern="*.dylib*", dst="lib", src="lib") 
    #     self.copy(pattern="*.so*", dst="lib", src="lib") 
       
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def package(self):
        pass

    def package_info(self):
        pass
