#include "rasterizer.h"
#include <algorithm>
#include <execution>
#include <iostream>
#include <list>
#include <thread>
#include <vector>
#include "JobSystem.h"
#include "common/core_assert.h"

namespace rs {

using namespace gfx;
using namespace std;

// config
static constexpr int tileSize = 200;

// barycentric
// P = uA + vB + wC (where w = 1 - u - v)
// P = uA + vB + C - uC - vC => (C - P) + u(A - C) + v(B - C) = 0
// uCA + vCB + PC = 0
// [u v 1] [CAx CBx PCx] = 0
// [u v 1] [CAy CBy PCy] = 0
// [u v 1] is the cross product

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

void initialize() {
}

void finalize() {
}

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
    int discarded = false;
};

/**
 * Perspective correct linear interpolation
 * https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation
 */
inline void ndcToViewport(vec4& position) {
    ASSERT(position.w != 0.0f);
    float invW = 1.0f / position.w;
    position.x *= invW;
    position.y *= invW;
    position.z *= invW;
    position.w = invW;

    position.x = 0.5f + 0.5f * position.x;
    position.y = 0.5f + 0.5f * position.y;
}

static inline OutTriangle processTriangle(const VSInput& vs_in0, const VSInput& vs_in1, const VSInput& vs_in2) {
    IVertexShader* vs = g_state.vs;
    VSOutput vs_out0 = vs->processVertex(vs_in0);
    VSOutput vs_out1 = vs->processVertex(vs_in1);
    VSOutput vs_out2 = vs->processVertex(vs_in2);

    // TODO: front plane clipping
    constexpr float t = 1.0f;
    if (vs_out0.position.z / vs_out0.position.w > t ||
        vs_out1.position.z / vs_out1.position.w > t ||
        vs_out2.position.z / vs_out2.position.w > t) {
        OutTriangle triangle;
        triangle.discarded = true;
        return triangle;
    }

    ndcToViewport(vs_out0.position);
    ndcToViewport(vs_out1.position);
    ndcToViewport(vs_out2.position);

    // face culling
    vec3 ab3d(vs_out0.position.x - vs_out1.position.x, vs_out0.position.y - vs_out1.position.y, vs_out0.position.z - vs_out1.position.z);
    vec3 ac3d(vs_out0.position.x - vs_out2.position.x, vs_out0.position.y - vs_out2.position.y, vs_out0.position.z - vs_out2.position.z);
    vec3 normal = cross(ab3d, ac3d);
    if (normal.z * g_state.cullFace < 0.0f) {
        OutTriangle triangle;
        triangle.discarded = true;
        return triangle;
    }

    return OutTriangle { vs_out0, vs_out1, vs_out2 };
}

static inline int tileNum(int tileSize, int length) {
    int rem = (length % tileSize) != 0;
    return (length / tileSize) + rem;
}

static void inline processFragment(OutTriangle& vs_out, int tx, int ty) {
    // prepare to go multi-threading
    RenderTarget* rt = g_state.rt;
    const int width = rt->m_depthBuffer.m_width;
    const int height = rt->m_depthBuffer.m_height;

    // note
    const int col = tileNum(tileSize, width);
    const int row = tileNum(tileSize, height);

    const VSOutput& vs_out0 = vs_out.p0;
    const VSOutput& vs_out1 = vs_out.p1;
    const VSOutput& vs_out2 = vs_out.p2;
    IVertexShader* vs = g_state.vs;
    IFragmentShader* fs = g_state.fs;
    const vec2 a(vs_out0.position.x * width, vs_out0.position.y * height);
    const vec2 b(vs_out1.position.x * width, vs_out1.position.y * height);
    const vec2 c(vs_out2.position.x * width, vs_out2.position.y * height);

    // discard if A, B and C are on the same line
    const vec2 BA = a - b;
    const vec2 CA = a - c;
    const vec2 CB = b - c;
    if (BA.x * CA.y == BA.y * CA.x) {
        return;
    }

    ColorBuffer& colorBuffer = rt->m_colorBuffer;
    DepthBuffer& depthBuffer = rt->m_depthBuffer;
    const uint32_t varyingFlags = vs->getVaryingFlags();

    const vec2 _min(tx * tileSize, ty * tileSize);
    const vec2 _max(
        glm::min(width - 1, (tx + 1) * tileSize),
        glm::min(height - 1, (ty + 1) * tileSize));
    const Box2 screenBox(_min, _max);
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

    // rasterization
    for (int y = int(triangleBox.min.y); y < triangleBox.max.y; ++y) {
        for (int x = int(triangleBox.min.x); x < triangleBox.max.x; ++x) {
            const vec2 p(x, y);
            const vec2 PC = c - p;
            vec3 uvw = cross(vec3(CA.x, CB.x, PC.x), vec3(CA.y, CB.y, PC.y));
            if (uvw.z == 0.0f) {
                continue;
            }
            uvw /= uvw.z;
            uvw.z -= (uvw.x + uvw.y);
            vec3 bCoord = uvw;

            static const float epsilon = glm::epsilon<float>();
            const float sum = bCoord.x + bCoord.y + bCoord.z;
            bool test = (bCoord.x >= 0.0f && bCoord.y >= 0.0f && bCoord.z >= 0.0f && std::abs(sum - 1.0f) <= epsilon);
            if (test == true) {
                // depth test
                VSOutput output;
                output.position = bCoord.x * vs_out0.position + bCoord.y * vs_out1.position + bCoord.z * vs_out2.position;
                output.position.z = 0.5f * output.position.z + 0.5f;  // normalize to [0, 1]
                const size_t index = (height - y - 1) * width + x;

                float depth = output.position.z;

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

static void drawArrayInternal(vector<OutTriangle>& trigs) {
    // remove invalid triangles
    trigs.erase(remove_if(trigs.begin(),
                          trigs.end(),
                          [](OutTriangle& trig) { return trig.discarded; }),
                trigs.end());

    RenderTarget* rt = g_state.rt;
    const int width = rt->m_depthBuffer.m_width;
    const int height = rt->m_depthBuffer.m_height;

    const int col = tileNum(tileSize, width);
    const int row = tileNum(tileSize, height);

    jobsystem::Context ctx;
    ctx.Dispatch(row, 1, [&](jobsystem::JobArgs args) {
        for (int c = 0; c < col; ++c) {
            for (OutTriangle& triangle : trigs) {
                processFragment(triangle, c, args.jobIndex);
            }
        }
    });

    ctx.Wait();
}

void drawArrays(size_t start, size_t count) {
    ASSERT(count % 3 == 0);
    ASSERT(start % 3 == 0);

    const int triangleCnt = int(count) / 3;
    vector<OutTriangle> outTriangles(triangleCnt);

    jobsystem::Context ctx;
    ctx.Dispatch(triangleCnt, 8, [&](jobsystem::JobArgs args) {
        const uint32_t idx = args.jobIndex;
        const VSInput* vertices = g_state.vertices;
        const VSInput& p0 = vertices[idx * 3 + 0];
        const VSInput& p1 = vertices[idx * 3 + 1];
        const VSInput& p2 = vertices[idx * 3 + 2];
        outTriangles[idx] = processTriangle(p0, p1, p2);
    });
    ctx.Wait();

    drawArrayInternal(outTriangles);
}

void drawElements(size_t start, size_t count) {
    ASSERT(count % 3 == 0);
    ASSERT(start % 3 == 0);

    const int triangleCnt = int(count) / 3;
    vector<OutTriangle> outTriangles(triangleCnt);

    jobsystem::Context ctx;
    ctx.Dispatch(triangleCnt, 8, [&](jobsystem::JobArgs args) {
        const uint32_t idx = args.jobIndex;
        const VSInput* vertices = g_state.vertices;
        const uint32_t* indices = g_state.indices;
        const VSInput& p0 = vertices[indices[idx * 3 + 0]];
        const VSInput& p1 = vertices[indices[idx * 3 + 1]];
        const VSInput& p2 = vertices[indices[idx * 3 + 2]];
        outTriangles[idx] = processTriangle(p0, p1, p2);
    });
    ctx.Wait();

    drawArrayInternal(outTriangles);
}

}  // namespace rs
