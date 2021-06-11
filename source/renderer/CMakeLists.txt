add_library(renderer
    Renderer.cpp
    RenderTarget.cpp
    Texture.cpp
)

add_library(sr::renderer ALIAS renderer)
