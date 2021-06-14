#include "rasterizer.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>
#include "common/core_assert.h"

namespace rs {

using namespace gfx;
using namespace std;

struct RenderState {
    gfx::Color clearColor = gfx::Color { 0, 0, 0, 255 };
    float clearDepth = 1.0f;
    IVertexShader* vs = nullptr;
    IFragmentShader* fs = nullptr;
    RenderTarget* rt = nullptr;

    const VSInput* vertices = nullptr;
    const uint32_t* indices = nullptr;
    float cullFace = 0.0f;
};

static RenderState g_state;

void initialize() {}

void finalize() {}

void setSize(int width, int height) {
    ASSERT(width > 0 && height > 0);
    g_state.rt->resize(width, height);
    g_state.rt->m_colorBuffer.clear(g_state.clearColor);
    g_state.rt->m_depthBuffer.clear(g_state.clearDepth);
}

void setClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    g_state.clearColor.r = r;
    g_state.clearColor.g = g;
    g_state.clearColor.b = b;
    g_state.clearColor.a = a;
}

void setClearDepth(float depth) {
    g_state.clearDepth = depth;
}

void clear(ClearFlags flags) {
    if (flags & ClearFlags::COLOR_BUFFER_BIT) {
        g_state.rt->m_colorBuffer.clear(g_state.clearColor);
    }
    if (flags & ClearFlags::DEPTH_BUFFER_BIT) {
        g_state.rt->m_depthBuffer.clear(g_state.clearDepth);
    }
}

void setCullState(CullState cullState) {
    g_state.cullFace = static_cast<float>(cullState);
}

void setVertexArray(const VSInput* vertices) { g_state.vertices = vertices; }
void setIndexArray(const unsigned int* indices) { g_state.indices = indices; }
void setVertexShader(IVertexShader* vs) { g_state.vs = vs; }
void setFragmentShader(IFragmentShader* fs) { g_state.fs = fs; }
void setRenderTarget(RenderTarget* renderTarget) { g_state.rt = renderTarget; }

struct OutTriangle {
    VSOutput p0, p1, p2;
};

/**
 * Perspective correct linear interpolation
 * https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation
 */
inline void ndcToViewport(vec4& position) {
    float invW = 1.0f / position.w;
    position.x *= invW;
    position.y *= invW;
    position.z *= invW;
    position.w = invW;

    position.x = 0.5f + 0.5f * position.x;
    position.y = 0.5f + 0.5f * position.y;
}

static inline void processTriangle(const VSInput& vs_in0, const VSInput& vs_in1, const VSInput& vs_in2, vector<OutTriangle>& outTriangles) {
    IVertexShader* vs = g_state.vs;
    VSOutput vs_out0 = vs->processVertex(vs_in0);
    VSOutput vs_out1 = vs->processVertex(vs_in1);
    VSOutput vs_out2 = vs->processVertex(vs_in2);

    // clipping
    // hack: discard if all 3 points are outside the viewport
    // todo: clip triangle properly
    {
        constexpr float t = -1.0f;
        if (vs_out0.position.z / vs_out0.position.w < t ||
            vs_out1.position.z / vs_out1.position.w < t ||
            vs_out2.position.z / vs_out2.position.w < t) {
            return;
        }
    }
    {
        constexpr float t = 1.0f;
        if (vs_out0.position.z / vs_out0.position.w > t ||
            vs_out1.position.z / vs_out1.position.w > t ||
            vs_out2.position.z / vs_out2.position.w > t) {
            return;
        }
    }
    {
        constexpr float t = -1.0f;
        if (vs_out0.position.x / vs_out0.position.w < t ||
            vs_out1.position.x / vs_out1.position.w < t ||
            vs_out2.position.x / vs_out2.position.w < t) {
            return;
        }
    }
    {
        constexpr float t = 1.0f;
        if (vs_out0.position.x / vs_out0.position.w > t ||
            vs_out1.position.x / vs_out1.position.w > t ||
            vs_out2.position.x / vs_out2.position.w > t) {
            return;
        }
    }

    ndcToViewport(vs_out0.position);
    ndcToViewport(vs_out1.position);
    ndcToViewport(vs_out2.position);

    // face culling
    vec3 ab3d(vs_out0.position.x - vs_out1.position.x, vs_out0.position.y - vs_out1.position.y, vs_out0.position.z - vs_out1.position.z);
    vec3 ac3d(vs_out0.position.x - vs_out2.position.x, vs_out0.position.y - vs_out2.position.y, vs_out0.position.z - vs_out2.position.z);
    vec3 normal = cross(ab3d, ac3d);
    if (normal.z * g_state.cullFace < 0.0f) {
        return;
    }

    outTriangles.emplace_back(OutTriangle { vs_out0, vs_out1, vs_out2 });
}

static void inline processFragment(OutTriangle& vs_out) {
    RenderTarget* rt = g_state.rt;
    const int width = rt->m_depthBuffer.m_width;
    const int height = rt->m_depthBuffer.m_height;

    IVertexShader* vs = g_state.vs;
    IFragmentShader* fs = g_state.fs;
    VSOutput& vs_out0 = vs_out.p0;
    VSOutput& vs_out1 = vs_out.p1;
    VSOutput& vs_out2 = vs_out.p2;

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
    if (!intersect) {
        return;
    }

    // discard if A, B and C are on the same line
    const vec2 ab = b - a;
    const vec2 ac = c - a;
    if (ab.x * ac.y == ab.y * ac.x) {
        return;
    }

    ColorBuffer& colorBuffer = rt->m_colorBuffer;
    DepthBuffer& depthBuffer = rt->m_depthBuffer;

    // rasterization
    const uint32_t varyingFlags = vs->getVaryingFlags();
    for (int y = int(triangleBox.min.y); y < triangleBox.max.y; ++y) {
        for (int x = int(triangleBox.min.x); x < triangleBox.max.x; ++x) {
            // barycentric
            const vec2 p(x, y);
            const vec2 PC = c - p, CA = a - c, CB = b - c;
            vec3 uvw = cross(vec3(CA.x, CB.x, PC.x), vec3(CA.y, CB.y, PC.y));
            if (uvw.z == 0.0f) {
                continue;
            }
            uvw /= uvw.z;
            uvw.z -= (uvw.x + uvw.y);
            vec3 bCoord = uvw;

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

                if (depth >= 1.0f || depth <= .0f) {
                    continue;
                }
                if (depthBuffer.m_buffer[index] < depth) {
                    continue;
                }

                depthBuffer.m_buffer[index] = depth;
                // corrected barycentric coordinates
                float x = bCoord.x * vs_out0.position.w;
                float y = bCoord.y * vs_out1.position.w;
                float z = bCoord.z * vs_out2.position.w;
                float xyzSum = x + y + z;
                bCoord.x = x / xyzSum;
                bCoord.y = y / xyzSum;
                bCoord.z = z / xyzSum;

                if (varyingFlags & VARYING_NORMAL) {
                    output.normal = bCoord.x * vs_out0.normal + bCoord.y * vs_out1.normal + bCoord.z * vs_out2.normal;
                }
                if (varyingFlags & VARYING_COLOR) {
                    output.color = bCoord.x * vs_out0.color + bCoord.y * vs_out1.color + bCoord.z * vs_out2.color;
                }
                if (varyingFlags & VARYING_UV) {
                    // NOTE: slow here! probably due to unaligned simd operation
                    output.uv = bCoord.x * vs_out0.uv + bCoord.y * vs_out1.uv + bCoord.z * vs_out2.uv;
                }
                if (varyingFlags & VARYING_WORLD_POSITION) {
                    output.worldPosition = bCoord.x * vs_out0.worldPosition + bCoord.y * vs_out1.worldPosition + bCoord.z * vs_out2.worldPosition;
                }

                // fragment shader
                colorBuffer.m_buffer[index] = fs->processFragment(output);
            }
        }
    }
}

void drawArrays(size_t start, size_t count) {
    ASSERT(count % 3 == 0);
    ASSERT(start % 3 == 0);

    const VSInput* vertices = g_state.vertices;
    vector<OutTriangle> outTriangles(count / 3);
    for (size_t i = start; i < start + count;) {
        const VSInput& p0 = vertices[i++];
        const VSInput& p1 = vertices[i++];
        const VSInput& p2 = vertices[i++];
        processTriangle(p0, p1, p2, outTriangles);
    }

    for (OutTriangle& triangle : outTriangles) {
        processFragment(triangle);
    }
}

void drawElements(size_t start, size_t count) {
    ASSERT(count % 3 == 0);
    ASSERT(start % 3 == 0);

    const VSInput* vertices = g_state.vertices;
    const uint32_t* indices = g_state.indices;
    vector<OutTriangle> outTriangles(count / 3);
    for (size_t i = start; i < start + count;) {
        const VSInput& p0 = vertices[indices[i++]];
        const VSInput& p1 = vertices[indices[i++]];
        const VSInput& p2 = vertices[indices[i++]];
        processTriangle(p0, p1, p2, outTriangles);
    }

    for (OutTriangle& triangle : outTriangles) {
        processFragment(triangle);
    }
}

}  // namespace rs
