#pragma once
#include "ShaderProgram.h"
#include "Texture.h"

namespace hguo {

class TextureVs : public IVertexShader {
   public:
    TextureVs()
        : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput& input) override {
        VSOutput vs_output;
        vs_output.position = input.position;
        vs_output.uv = gfx::vec2(input.position);
        vs_output.uv += 0.5f;
        vs_output.uv.y = 1.0f - vs_output.uv.y;
        vs_output.position = PVM * vs_output.position;
        return vs_output;
    }

   public:
    gfx::mat4 PVM;

   private:
    static const unsigned int sVaryingFlags = VARYING_UV;
};

class TextureFs : public IFragmentShader {
   public:
    virtual gfx::Color processFragment(const VSOutput& input) override {
        gfx::Color color = m_cubeTexture->sample(input.uv);
        return color;
    }

   public:
    const Texture* m_cubeTexture;
};

}  // namespace hguo
