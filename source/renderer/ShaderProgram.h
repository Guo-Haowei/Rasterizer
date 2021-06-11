#pragma once
#include "Texture.h"
#include "common/linalg.h"

namespace hguo {

enum VaryingFlag : unsigned int {
    VARYING_COLOR = 1u << 0,
    VARYING_NORMAL = 1u << 1,
    VARYING_UV = 1u << 2,
    VARYING_WORLD_POSITION = 1u << 3,
    VARYING_LIGHT_SPACE_POSITION = 1u << 4,
};

struct VSInput {
    gfx::vec4 position;
    gfx::vec4 normal;
    gfx::vec2 uv;
    gfx::vec3 color;
    gfx::ivec4 boneId;
    gfx::vec4 weights;
};

struct VSOutput {
    gfx::vec4 position;
    gfx::vec4 worldPosition;
    gfx::vec4 normal;
    gfx::vec4 lightSpacePosition;
    gfx::vec2 uv;
    gfx::vec3 color;
};

class IVertexShader {
   public:
    virtual VSOutput processVertex(const VSInput& input) = 0;

    unsigned int getVaryingFlags() const { return m_varyingFlags; }

   protected:
    IVertexShader() {}

    IVertexShader(unsigned int varyingFlags) : m_varyingFlags(varyingFlags) {}

    const unsigned int m_varyingFlags = 0;
};

class IFragmentShader {
   public:
    virtual gfx::Color processFragment(const VSOutput& input) = 0;
};

}  // namespace hguo
