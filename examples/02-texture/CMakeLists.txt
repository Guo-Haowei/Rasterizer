# triangle test
add_executable(02-texture texture_test.cpp)

target_link_libraries(02-texture PRIVATE
    application
    common
    loader
    renderer
)

target_compile_definitions(02-texture PRIVATE
    -DASSET_DIR="${PROJECT_SOURCE_DIR}/assets/"
)
