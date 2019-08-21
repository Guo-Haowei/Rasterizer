# triangle test
add_executable(shadowTest ShadowTest.cpp)

target_link_libraries(shadowTest PRIVATE sr::viewer sr::utilities)

target_compile_definitions(shadowTest PRIVATE
    -DASSET_DIR="${PROJECT_SOURCE_DIR}/assets/"
)
