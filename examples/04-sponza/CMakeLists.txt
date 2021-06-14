# triangle test
add_executable(04-sponza sponza_test.cpp)

target_link_libraries(04-sponza PRIVATE
    application
    common
    loader
    renderer
)

target_compile_definitions(04-sponza PRIVATE
    -DASSET_DIR="${PROJECT_SOURCE_DIR}/assets/"
)
