# triangle test
add_executable(textureTest TextureTest.cpp)

target_link_libraries(textureTest PRIVATE
    sr::viewer
    sr::utilities
)

target_compile_definitions(textureTest PRIVATE
    -DASSET_DIR="${PROJECT_SOURCE_DIR}/assets/"
)
