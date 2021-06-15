# triangle test
add_executable(04-model model_test.cpp)

target_link_libraries(04-model PRIVATE
    application
    common
    loader
    renderer
)

target_compile_definitions(04-model PRIVATE
    -DASSET_DIR="${PROJECT_SOURCE_DIR}/assets/"
)
