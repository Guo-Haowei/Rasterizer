# triangle test
add_executable(animationTest AnimationTest.cpp)

target_link_libraries(animationTest PRIVATE sr::viewer sr::utilities)

target_compile_definitions(animationTest PRIVATE
    -DASSET_DIR="${PROJECT_SOURCE_DIR}/assets/"
)
