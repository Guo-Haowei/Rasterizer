#pragma once
#include "../Maths.h"
#include "../ShaderProgram.h"

namespace hguo {

class ModelVS : public IVertexShader {
   public:
    ModelVS() : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput& input) override {
        VSOutput vs_output;
        vs_output.worldPosition = M * input.position;
        vs_output.position = PV * (vs_output.worldPosition);
        vs_output.normal = M * input.normal;
        vs_output.lightSpacePosition = lightPV * (vs_output.worldPosition);
        return vs_output;
    }

   public:
    Matrix4 PV;
    Matrix4 M;
    Matrix4 lightPV { Matrix4::Identity };

   private:
    static const unsigned int sVaryingFlags = VARYING_NORMAL | VARYING_WORLD_POSITION | VARYING_LIGHT_SPACE_POSITION;
};

}  // namespace hguo
