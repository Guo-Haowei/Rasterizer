add_subdirectory(common)
add_subdirectory(rasterizer)

if (WIN32)
    add_library(application
        example_base.cpp
        application_win32.cpp
        main.cpp
    )
elseif (APPLE)
    add_library(application
        example_base.cpp
        application_macos.cpp
        main.cpp
    )
endif ()
