add_library(viewer
    main.cpp
    BaseApp.cpp
)

add_library(sr::viewer ALIAS viewer)

target_link_libraries(viewer PUBLIC
    sr::renderer
)

target_include_directories(viewer PUBLIC
    ../renderer
)
