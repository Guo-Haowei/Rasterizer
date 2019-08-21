add_library(renderer
    Maths.cpp
    Renderer.cpp
    RenderTarget.cpp
    Texture.cpp
)

target_link_libraries(renderer PUBLIC three::math)

add_library(sr::renderer ALIAS renderer)
