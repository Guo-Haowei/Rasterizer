#pragma once
#include "../Maths.h"
#include "../ShaderProgram.h"

namespace hguo {

class ModelDepthVS : public IVertexShader {
   public:
    virtual VSOutput processVertex(const VSInput& input) override {
        VSOutput vs_output;
        vs_output.worldPosition = M * input.position;
        vs_output.position = PV * (vs_output.worldPosition);
        return vs_output;
    }

   public:
    Matrix4 PV;
    Matrix4 M;
};

}  // namespace hguo
