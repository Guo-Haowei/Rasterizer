# triangle test
add_executable(modelTest ModelTest.cpp)

target_link_libraries(modelTest PRIVATE sr::viewer sr::utilities)

target_compile_definitions(modelTest PRIVATE
    -DASSET_DIR="${PROJECT_SOURCE_DIR}/assets/"
)
