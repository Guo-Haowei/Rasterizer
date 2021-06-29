#pragma once
#include "common/linalg.h"
#include "sampler.h"

namespace rs {

enum VaryingFlag : unsigned int {
    VARYING_COLOR = 1u << 0,
    VARYING_NORMAL = 1u << 1,
    VARYING_UV = 1u << 2,
    VARYING_WORLD_POSITION = 1u << 3,
};

struct alignas(16) VSInput {
    gfx::vec4 position;
    gfx::vec4 normal;
    gfx::ivec4 boneId;
    gfx::vec4 weights;
    gfx::vec2 uv;
};

struct alignas(16) VSOutput {
    gfx::vec4 position;
    gfx::vec4 worldPosition;
    gfx::vec4 normal;
    gfx::vec4 color;
    gfx::vec2 uv;
};

class IVertexShader {
   public:
    virtual VSOutput processVertex(const VSInput& input) = 0;

    unsigned int getVaryingFlags() const { return m_varyingFlags; }

   protected:
    IVertexShader() {}

    IVertexShader(unsigned int varyingFlags)
        : m_varyingFlags(varyingFlags) {}

    const unsigned int m_varyingFlags = 0;
};

class IFragmentShader {
   public:
    virtual gfx::Color processFragment(const VSOutput& input) = 0;
};

}  // namespace rs
