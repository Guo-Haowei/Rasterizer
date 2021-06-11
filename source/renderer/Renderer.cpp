#include "Renderer.h"
#include <algorithm>
#include <iostream>
#include "common/core_assert.h"

namespace hguo {

using namespace gfx;

/**
 * P = uA + vB + wC (where w = 1 - u - v)
 * P = uA + vB + C - uC - vC => (C - P) + u(A - C) + v(B - C) = 0
 * uCA + vCB + PC = 0
 * [u v 1] [CAx CBx PCx] = 0
 * [u v 1] [CAy CBy PCy] = 0
 * [u v 1] is the cross product
 */
static vec3 barycentric(const vec2& a, const vec2& b, const vec2& c, const vec2& p) {
    const vec2 PC = c - p, CA = a - c, CB = b - c;
    vec3 uvw = cross(vec3(CA.x, CB.x, PC.x), vec3(CA.y, CB.y, PC.y));
    /**
     * what if uvw.z is 0?
     * CA.x * CB.y = CA.y * CB.x == 0 => CA.y / CA.x == CB.y / CB.x
     * CA // CB => A, B and C are on the same line
     */
    ASSERT(uvw.z != 0.0f);
    uvw /= uvw.z;
    uvw.z -= (uvw.x + uvw.y);
    return uvw;
}

Renderer::Renderer() {
}

void Renderer::setSize(int width, int height) {
    ASSERT(width > 0 && height > 0);
    m_pRenderTarget->resize(width, height);
    m_pRenderTarget->m_colorBuffer.clear(m_clearColor);
    m_pRenderTarget->m_depthBuffer.clear(m_clearDepth);
}

void Renderer::setClearColor(unsigned r, unsigned g, unsigned b, unsigned a) {
    m_clearColor.r = r;
    m_clearColor.g = g;
    m_clearColor.b = b;
    m_clearColor.a = a;
}

void Renderer::setClearDepth(float depth) {
    m_clearDepth = depth;
}

void Renderer::clear(ClearFlags flags) {
    if (flags & ClearFlags::COLOR_BUFFER_BIT)
        m_pRenderTarget->m_colorBuffer.clear(m_clearColor);
    if (flags & ClearFlags::DEPTH_BUFFER_BIT)
        m_pRenderTarget->m_depthBuffer.clear(m_clearDepth);
}

void Renderer::setCullState(CullState cullState) {
    m_cullFaceFactor = static_cast<float>(cullState);
}

void Renderer::drawArrays(size_t start, size_t count) {
    ASSERT(count % 3 == 0);
    ASSERT(start % 3 == 0);

    for (size_t i = start; i < start + count;) {
        const VSInput& vs1 = m_pVertices[i++];
        const VSInput& vs2 = m_pVertices[i++];
        const VSInput& vs3 = m_pVertices[i++];
        pipeline(vs1, vs2, vs3);
    }
}

void Renderer::drawElements(size_t start, size_t count) {
    ASSERT(count % 3 == 0);
    ASSERT(start % 3 == 0);

    for (size_t i = start; i < start + count;) {
        const VSInput& vs1 = m_pVertices[m_pIndices[i++]];
        const VSInput& vs2 = m_pVertices[m_pIndices[i++]];
        const VSInput& vs3 = m_pVertices[m_pIndices[i++]];
        pipeline(vs1, vs2, vs3);
    }
}

/**
 * Perspective correct linear interpolation
 * https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation
 */
VSOutput Renderer::ndcToViewport(const VSInput& vs_in) {
    VSOutput vs_out = m_pVertexShader->processVertex(vs_in);
    float invW = 1.0f / vs_out.position.w;
    vs_out.position.x *= invW;
    vs_out.position.y *= invW;
    vs_out.position.z *= invW;
    vs_out.position.w = invW;

    vs_out.position.x = 0.5f + 0.5f * vs_out.position.x;
    vs_out.position.y = 0.5f + 0.5f * vs_out.position.y;
    // revert z
    return vs_out;
}

void Renderer::pipeline(const VSInput& vs_in0, const VSInput& vs_in1, const VSInput& vs_in2) {
    // process vertex
    VSOutput vs_out0 = ndcToViewport(vs_in0);
    VSOutput vs_out1 = ndcToViewport(vs_in1);
    VSOutput vs_out2 = ndcToViewport(vs_in2);

    int width = m_pRenderTarget->m_depthBuffer.m_width;
    int height = m_pRenderTarget->m_depthBuffer.m_height;

    const vec2 a(vs_out0.position.x * width, vs_out0.position.y * height);
    const vec2 b(vs_out1.position.x * width, vs_out1.position.y * height);
    const vec2 c(vs_out2.position.x * width, vs_out2.position.y * height);

    const Box2 screenBox(vec2(0.0f), vec2(width - 1, height - 1));
    Box2 triangleBox {};
    triangleBox.expandPoint(a);
    triangleBox.expandPoint(b);
    triangleBox.expandPoint(c);
    triangleBox.unionBox(screenBox);

    bool intersect = triangleBox.isValid();
    // discard if not intersect
    if (!intersect)
        return;

    // discard if A, B and C are on the same line
    const vec2 ab = b - a;
    const vec2 ac = c - a;
    if (ab.x * ac.y == ab.y * ac.x)
        return;

    // face culling
    vec3 ab3d(vs_out0.position.x - vs_out1.position.x, vs_out0.position.y - vs_out1.position.y, vs_out0.position.z - vs_out1.position.z);
    vec3 ac3d(vs_out0.position.x - vs_out2.position.x, vs_out0.position.y - vs_out2.position.y, vs_out0.position.z - vs_out2.position.z);
    vec3 normal = cross(ab3d, ac3d);
    if (normal.z * m_cullFaceFactor < 0.0f)
        return;

    ColorBuffer& colorBuffer = m_pRenderTarget->m_colorBuffer;
    DepthBuffer& depthBuffer = m_pRenderTarget->m_depthBuffer;

    // rasterization
    const unsigned int varyingFlags = m_pVertexShader->getVaryingFlags();
    // check range?
    for (int y = int(triangleBox.min.y); y < triangleBox.max.y; ++y) {
        for (int x = int(triangleBox.min.x); x < triangleBox.max.x; ++x) {
            vec3 bCoord = barycentric(a, b, c, vec2(x, y));
            float sum = bCoord.x + bCoord.y + bCoord.z;
            // TODO: refactor
            static const float epsilon = 0.00003f;
            bool test = (bCoord.x >= 0.0f && bCoord.y >= 0.0f && bCoord.z >= 0.0f && std::abs(sum - 1.0f) <= epsilon);
            if (test == true) {
                // depth test
                VSOutput output;
                output.position = bCoord.x * vs_out0.position + bCoord.y * vs_out1.position + bCoord.z * vs_out2.position;
                output.position.z = 0.5f * output.position.z + 0.5f;  // normalize to [0, 1]
                const size_t index = (height - y - 1) * width + x;

                float depth = output.position.z;

                if (depthBuffer.m_buffer[index] < depth)
                    continue;

                depthBuffer.m_buffer[index] = depth;
                {
                    // corrected barycentric coordinates
                    float x = bCoord.x * vs_out0.position.w;
                    float y = bCoord.y * vs_out1.position.w;
                    float z = bCoord.z * vs_out2.position.w;
                    float xyzSum = x + y + z;
                    bCoord.x = x / xyzSum;
                    bCoord.y = y / xyzSum;
                    bCoord.z = z / xyzSum;
                }

                if (varyingFlags & VARYING_NORMAL) {
                    output.normal = bCoord.x * vs_out0.normal + bCoord.y * vs_out1.normal + bCoord.z * vs_out2.normal;
                }
                if (varyingFlags & VARYING_COLOR) {
                    output.color = bCoord.x * vs_out0.color + bCoord.y * vs_out1.color + bCoord.z * vs_out2.color;
                }
                if (varyingFlags & VARYING_UV) {
                    output.uv = bCoord.x * vs_out0.uv + bCoord.y * vs_out1.uv + bCoord.z * vs_out2.uv;
                }
                if (varyingFlags & VARYING_WORLD_POSITION) {
                    output.worldPosition = bCoord.x * vs_out0.worldPosition + bCoord.y * vs_out1.worldPosition + bCoord.z * vs_out2.worldPosition;
                }
                if (varyingFlags & VARYING_LIGHT_SPACE_POSITION) {
                    output.lightSpacePosition = bCoord.x * vs_out0.lightSpacePosition + bCoord.y * vs_out1.lightSpacePosition + bCoord.z * vs_out2.lightSpacePosition;
                }

                // fragment shader
                colorBuffer.m_buffer[index] = m_pFragmentShader->processFragment(output);
            }
        }
    }
}

}  // namespace hguo