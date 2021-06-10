# triangle test
add_executable(triangleTest TriangleTest.cpp)

target_link_libraries(triangleTest PRIVATE
    common
    sr::viewer
)
