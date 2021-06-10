cmake_minimum_required(VERSION 3.0)
project(SoftwareRenderer)
set(CMAKE_CXX_STANDARD 17)

# glfw
message("Configuring glfw...")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
include_directories(external/glfw/include)
add_subdirectory(external/glfw)

# glad
message("Configuring glad...")
include_directories(external/glad/include)
add_subdirectory(external/glad)

# three.math
message("Configuring three.math...")
include_directories(source/three.math/)
add_subdirectory(source/three.math/)

# imgui
message("Configuring imgui...")
include_directories(external/imgui/)
add_library(imgui
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui.cpp
)

# tinygltf
message("Configuring tinygltf...")
include_directories(external/tinygltf/)

add_library(imgui::imgui ALIAS imgui)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(source)
