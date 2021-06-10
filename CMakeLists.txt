cmake_minimum_required(VERSION 3.0)
project(SoftwareRenderer)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# three.math
message("Configuring three.math...")
include_directories(source/three.math/)
add_subdirectory(source/three.math/)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(source)
