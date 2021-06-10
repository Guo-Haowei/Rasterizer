#pragma once
#include <assert.h>
#include "../Maths.h"
#include "../ShaderProgram.h"
#include "../Texture.h"

namespace hguo {

bool calculateShadow(const Vector4& lightSpacePosition, const DepthBuffer* depthBuffer) {
    Vector3 projectCoords = Vector3(lightSpacePosition);
    projectCoords *= (1.0f / lightSpacePosition.w);
    projectCoords = 0.5f * projectCoords + 0.5f;

    float closestDepth = depthBuffer->sample(Vector2(projectCoords));
    float currentDepth = projectCoords.z;
    float bias = 0.005f;
    bool inShadow = currentDepth > closestDepth + bias;
    return inShadow;
}

class PhongWithShadowFS : public IFragmentShader {
   public:
    virtual Color processFragment(const VSOutput& input) override {
        bool inShadow = calculateShadow(input.lightSpacePosition, m_depthBuffer);
        if (inShadow)
            return Color { 0, 0, 0, 255 };

        const Vector3 normal = three::normalize(Vector3(input.normal));
        const Vector3 worldPosition = Vector3(input.worldPosition);
        const Vector3 lightDirection = three::normalize(m_lightPosition - worldPosition);
        // ambient
        float ambient = 0.0f;
        // diffuse
        float dot = three::dot(lightDirection, normal);
        float diffusePower = std::max(dot, 0.0f);

        // specular
        float specularPower = 0.0f;

        float l = std::min(ambient + diffusePower + specularPower, 1.0f);
        Color color = Color { 255, 255, 255, 255 };
        Color finalColor;
        finalColor.r = static_cast<unsigned char>(l * color.r);
        finalColor.g = static_cast<unsigned char>(l * color.g);
        finalColor.b = static_cast<unsigned char>(l * color.b);
        finalColor.a = 255;
        return finalColor;
    }

   public:
    Vector3 m_lightPosition { 0, 3, 3 };
    Vector3 cameraPosition;
    int shininess = 1;
    const DepthBuffer* m_depthBuffer = nullptr;
};

}  // namespace hguo
