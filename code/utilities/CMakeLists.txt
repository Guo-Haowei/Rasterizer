add_library(utilities
    Loader.cpp
)

target_include_directories(utilities PRIVATE
    ${PROJECT_SOURCE_DIR}/external/stb/
)

if (WIN32)
    target_include_directories(utilities PUBLIC ${PROJECT_SOURCE_DIR}/external/assimp/include)
    set(lib_assimp_release "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Release/assimp-vc142-mt.lib")
    set(lib_irrxmld_release "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Release/IrrXML.lib")
    set(lib_zlib_release "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Release/zlibstatic.lib")
    target_link_libraries(utilities optimized ${lib_assimp_release} ${lib_irrxmld_release} ${lib_zlib_release})

    set(lib_assimp_debug "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Debug/assimp-vc142-mtd.lib")
    set(lib_irrxmld_debug "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Debug/IrrXMLd.lib")
    set(lib_zlib_debug "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Debug/zlibstaticd.lib")
    target_link_libraries(utilities debug ${lib_assimp_debug} ${lib_irrxmld_debug} ${lib_zlib_debug})
elseif (APPLE)
    find_package(ZLIB REQUIRED)
    target_include_directories(utilities PUBLIC ${PROJECT_SOURCE_DIR}/external/assimp/include)
    set(lib_assimp_release "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Release/libassimp.a")
    set(lib_irrxmld_release "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Release/libIrrXML.a")
    target_link_libraries(utilities optimized ${lib_assimp_release} ${lib_irrxmld_release} ZLIB::ZLIB)

    set(lib_assimp_debug "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Debug/libassimpd.a")
    set(lib_irrxmld_debug "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Debug/libIrrXMLd.a")
    target_link_libraries(utilities debug ${lib_assimp_debug} ${lib_irrxmld_debug} ZLIB::ZLIB)
endif ()

add_library(sr::utilities ALIAS utilities)
