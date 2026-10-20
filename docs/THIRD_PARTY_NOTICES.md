# Third-party notices for the portable candidate

The executable uses GLFW, GLAD, Dear ImGui, spdlog and the vendored
nlohmann/json header. Their license texts are included under `licenses/` in
the package with separate filenames. GoogleTest is a development/test
dependency and is not in the runtime package. The MSVC runtime DLLs are
redistributed from the installed Visual Studio toolchain; the product does
not include an installer or system runtime replacement. OpenGL and Windows
system DLLs are provided by the operating system.

The package does not incorporate artwork from the UI/UX v0.3 reference
images. Those images remain design inputs, not distributable product assets.
