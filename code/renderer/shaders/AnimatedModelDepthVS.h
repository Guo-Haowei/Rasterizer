#pragma once
#include "../Maths.h"
#include "../ShaderProgram.h"

namespace hguo {

class AnimatedModelDepthVS : public IVertexShader
{
public:
    virtual VSOutput processVertex(const VSInput& input) override
    {
        VSOutput vs_output;
        Matrix4 boneTransform = boneMatrices[input.boneId.x] * input.weights.x;
        boneTransform += boneMatrices[input.boneId.y] * input.weights.y;
        boneTransform += boneMatrices[input.boneId.z] * input.weights.z;
        boneTransform += boneMatrices[input.boneId.w] * input.weights.w;
        Matrix4 worldTransform = M * boneTransform;
        vs_output.worldPosition = worldTransform * input.position;
        vs_output.position = PV * (vs_output.worldPosition);
        return vs_output;
    }

public:
    Matrix4 PV;
    Matrix4 M;
    std::vector<Matrix4> boneMatrices;
};


} // namespace hguo


