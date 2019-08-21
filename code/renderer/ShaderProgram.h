#pragma once
#include "Maths.h"
#include "Texture.h"

namespace hguo {

enum VaryingFlag : unsigned int
{
    VARYING_COLOR = 1u << 0,
    VARYING_NORMAL = 1u << 1,
    VARYING_UV = 1u << 2,
    VARYING_WORLD_POSITION = 1u << 3,
    VARYING_LIGHT_SPACE_POSITION = 1u << 4,
};

struct VSInput
{
    Vector4  position;
    Vector4  normal;
    Vector2  uv;
    Vector3  color;
    Vector4i boneId;
    Vector4  weights;
};

struct VSOutput
{
    Vector4  position;
    Vector4  worldPosition;
    Vector4  normal;
    Vector4  lightSpacePosition;
    Vector2  uv;
    Vector3  color;
};

class IVertexShader
{
public:
    virtual VSOutput processVertex(const VSInput& input) = 0;

    unsigned int getVaryingFlags() const { return m_varyingFlags; }

protected:
    IVertexShader() {}

    IVertexShader(unsigned int varyingFlags) : m_varyingFlags(varyingFlags) { }

    const unsigned int m_varyingFlags = 0;
};

class IFragmentShader
{
public:
    virtual Color processFragment(const VSOutput& input) = 0;
};

} // namespace hguo
