# triangle test
add_executable(01-triangle triangle_test.cpp)

target_link_libraries(01-triangle PRIVATE
    application
    common
    renderer
)
