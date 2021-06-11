#pragma once
#include <vector>
#include "RenderTarget.h"
#include "ShaderProgram.h"
#include "common/linalg.h"

namespace rs {

struct Stats {
    int backfaceCulled;
};

class Renderer {
   public:
    enum ClearFlags : unsigned int {
        COLOR_BUFFER_BIT = 1 << 0,
        DEPTH_BUFFER_BIT = 1 << 1,
        COLOR_DEPTH_BUFFER_BIT = COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT,
    };

    enum CullState : int {
        NONE = 0,
        BACK_FACE = 1,
        FRONT_FACE = -1
    };

   public:
    Renderer();

    void setSize(int width, int height);

    void setClearColor(unsigned r, unsigned g, unsigned b, unsigned a);

    void setClearDepth(float depth);

    void clear(ClearFlags flags);

    void setCullState(CullState cullState);

    void setVertexArray(const VSInput* vertices) { m_pVertices = vertices; }

    void setIndexArray(const unsigned int* indices) { m_pIndices = indices; }

    void drawArrays(size_t start, size_t count);

    void drawElements(size_t start, size_t count);

    inline void setVertexShader(IVertexShader* vs) { m_pVertexShader = vs; }

    inline void setFragmentShader(IFragmentShader* fs) { m_pFragmentShader = fs; }

    inline void setRenderTarget(RenderTarget* renderTarget) { m_pRenderTarget = renderTarget; }

   private:
    VSOutput ndcToViewport(const VSInput& vs_in);

    void pipeline(const VSInput& vs_in0, const VSInput& vs_in1, const VSInput& vs_in2);

   private:
    gfx::Color m_clearColor = gfx::Color { 0, 0, 0, 255 };
    float m_clearDepth = 1.0f;
    IVertexShader* m_pVertexShader = nullptr;
    IFragmentShader* m_pFragmentShader = nullptr;
    RenderTarget* m_pRenderTarget = nullptr;

    const VSInput* m_pVertices = nullptr;
    const unsigned int* m_pIndices = nullptr;
    float m_cullFaceFactor = 0.0f;
};

}  // namespace rs