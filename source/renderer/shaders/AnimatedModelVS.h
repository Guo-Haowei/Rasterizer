#pragma once
#include "../ShaderProgram.h"
#include "common/linalg.h"

namespace hguo {

class AnimatedModelVS : public IVertexShader {
   public:
    AnimatedModelVS()
        : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput& input) override {
        VSOutput vs_output;
        gfx::mat4 boneTransform = boneMatrices[input.boneId.x] * input.weights.x;
        boneTransform += (boneMatrices[input.boneId.y] * input.weights.y);
        boneTransform += (boneMatrices[input.boneId.z] * input.weights.z);
        boneTransform += (boneMatrices[input.boneId.w] * input.weights.w);
        gfx::mat4 worldTransform = M * boneTransform;
        vs_output.worldPosition = worldTransform * input.position;
        vs_output.position = PV * (vs_output.worldPosition);
        vs_output.normal = worldTransform * input.normal;
        vs_output.uv = input.uv;
        return vs_output;
    }

   public:
    gfx::mat4 PV;
    gfx::mat4 M;
    std::vector<gfx::mat4> boneMatrices;

   private:
    static const unsigned int sVaryingFlags = VARYING_NORMAL | VARYING_UV | VARYING_WORLD_POSITION;
};

}  // namespace hguo
