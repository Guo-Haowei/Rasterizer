#pragma once
#include "../ShaderProgram.h"
#include "common/linalg.h"

namespace hguo {

class TexturePhongFS : public IFragmentShader {
   public:
    virtual gfx::Color processFragment(const VSOutput& input) override {
        const gfx::vec3 normal = gfx::normalize(gfx::vec3(input.normal));
        const gfx::vec3 worldPosition(input.worldPosition);
        const gfx::vec3 lightDirection = gfx::normalize(m_lightPosition - worldPosition);
        float dot = gfx::dot(lightDirection, normal);
        float diffusePower = gfx::max(dot, 0.0f);
        gfx::Color textureColor = m_texture->sample(input.uv);
        gfx::Color finalColor;
        finalColor.r = static_cast<unsigned char>(diffusePower * textureColor.r);
        finalColor.g = static_cast<unsigned char>(diffusePower * textureColor.g);
        finalColor.b = static_cast<unsigned char>(diffusePower * textureColor.b);
        finalColor.a = 255;
        return finalColor;
    }

   public:
    gfx::vec3 m_lightPosition { 0, 3, 3 };
    const Texture* m_texture = nullptr;
};

}  // namespace hguo
