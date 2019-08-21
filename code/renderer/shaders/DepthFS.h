#pragma once
#include "../Maths.h"
#include "../ShaderProgram.h"

namespace hguo {

class DepthFS : public IFragmentShader
{
public:
    virtual Color processFragment(const VSOutput& input) override
    {
        float depth = input.position.z; // z is between [-1, 1]
        depth = 0.5f * (depth + 1.0f);
        unsigned char depthColor = static_cast<unsigned char>(depth * 255.99f);
        return Color { depthColor, depthColor, depthColor, 255 };
    }

public:
};

} // namespace hguo

