#pragma once
#include "../Maths.h"
#include "../ShaderProgram.h"

namespace hguo {

class TexturePhongFS : public IFragmentShader {
   public:
    virtual Color processFragment(const VSOutput& input) override {
        const Vector3 normal = three::normalize(Vector3(input.normal));
        const Vector3 worldPosition(input.worldPosition);
        const Vector3 lightDirection = three::normalize(m_lightPosition - worldPosition);
        float dot = three::dot(lightDirection, normal);
        float diffusePower = std::max(dot, 0.0f);
        Color textureColor = m_texture->sample(input.uv);
        Color finalColor;
        finalColor.r = static_cast<unsigned char>(diffusePower * textureColor.r);
        finalColor.g = static_cast<unsigned char>(diffusePower * textureColor.g);
        finalColor.b = static_cast<unsigned char>(diffusePower * textureColor.b);
        finalColor.a = 255;
        return finalColor;
    }

   public:
    Vector3 m_lightPosition { 0, 3, 3 };
    const Texture* m_texture = nullptr;
};

}  // namespace hguo
