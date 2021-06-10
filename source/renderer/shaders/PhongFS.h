#pragma once
#include "../Maths.h"
#include "../ShaderProgram.h"

namespace hguo {

class PhongFS : public IFragmentShader {
   public:
    virtual Color processFragment(const VSOutput& input) override {
        const Vector3 normal = three::normalize(Vector3(input.normal));
        const Vector3 worldPosition = Vector3(input.worldPosition);
        const Vector3 lightDirection = three::normalize(m_lightPosition - worldPosition);
        // ambient
        float ambient = 0.0f;
        // diffuse
        float dot = three::dot(lightDirection, normal);
        float diffusePower = std::max(dot, 0.0f);
        // specular
        auto viewDirection = three::normalize(cameraPosition - worldPosition);
        auto reflectDirection = three::reflect(-lightDirection, normal);
        float angleBetweenViewAndReflect = three::dot(viewDirection, reflectDirection);
        float specularPower = std::pow(std::max(angleBetweenViewAndReflect, 0.0f), shininess);

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
};

}  // namespace hguo
