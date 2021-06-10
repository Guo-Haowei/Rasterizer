#pragma once
#include "../Maths.h"
#include "../ShaderProgram.h"

namespace hguo {

class AnimatedModelVS : public IVertexShader {
   public:
    AnimatedModelVS() : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput& input) override {
        VSOutput vs_output;
        Matrix4 boneTransform = boneMatrices[input.boneId.x] * input.weights.x;
        boneTransform += (boneMatrices[input.boneId.y] * input.weights.y);
        boneTransform += (boneMatrices[input.boneId.z] * input.weights.z);
        boneTransform += (boneMatrices[input.boneId.w] * input.weights.w);
        Matrix4 worldTransform = M * boneTransform;
        vs_output.worldPosition = worldTransform * input.position;
        vs_output.position = PV * (vs_output.worldPosition);
        vs_output.normal = worldTransform * input.normal;
        vs_output.uv = input.uv;
        return vs_output;
    }

   public:
    Matrix4 PV;
    Matrix4 M;
    std::vector<Matrix4> boneMatrices;

   private:
    static const unsigned int sVaryingFlags = VARYING_NORMAL | VARYING_UV | VARYING_WORLD_POSITION;
};

}  // namespace hguo
