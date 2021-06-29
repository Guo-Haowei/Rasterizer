add_library(loader
    loader.cpp
)

target_include_directories(loader PRIVATE
    ${PROJECT_SOURCE_DIR}/third_party/stb/
    ${PROJECT_SOURCE_DIR}/third_party/assimp/include/
)

set(lib_assimp_release "${PROJECT_SOURCE_DIR}/third_party/assimp/build/lib/Release/assimp-vc142-mt.lib")
set(lib_irrxmld_release "${PROJECT_SOURCE_DIR}/third_party/assimp/build/lib/Release/IrrXML.lib")
set(lib_zlib_release "${PROJECT_SOURCE_DIR}/third_party/assimp/build/lib/Release/zlibstatic.lib")
target_link_libraries(loader optimized ${lib_assimp_release} ${lib_irrxmld_release} ${lib_zlib_release})

set(lib_assimp_debug "${PROJECT_SOURCE_DIR}/third_party/assimp/build/lib/Debug/assimp-vc142-mtd.lib")
set(lib_irrxmld_debug "${PROJECT_SOURCE_DIR}/third_party/assimp/build/lib/Debug/IrrXMLd.lib")
set(lib_zlib_debug "${PROJECT_SOURCE_DIR}/third_party/assimp/build/lib/Debug/zlibstaticd.lib")
target_link_libraries(loader debug ${lib_assimp_debug} ${lib_irrxmld_debug} ${lib_zlib_debug})
