# triangle test
add_executable(03-animation animation_test.cpp)

target_link_libraries(03-animation PRIVATE
    application
    common
    loader
    renderer
)

target_compile_definitions(03-animation PRIVATE
    -DASSET_DIR="${PROJECT_SOURCE_DIR}/../assets/"
)
