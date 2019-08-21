add_library(viewer
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    main.cpp
    BaseApp.cpp
)

add_library(sr::viewer ALIAS viewer)

target_link_libraries(viewer PUBLIC
    glad
    glfw
    imgui::imgui
    sr::renderer
)

target_include_directories(viewer PUBLIC
    ../renderer
)
