add_subdirectory(common)
add_subdirectory(rasterizer)

add_library(application
    example_base.cpp
    application_win32.cpp
    main.cpp
)
