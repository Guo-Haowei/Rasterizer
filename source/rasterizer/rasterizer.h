#pragma once
#include <cstdint>
#include <vector>
#include "common/linalg.h"
#include "render_target.h"
#include "shader_program.h"

namespace rs {

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

void initialize();
void finalize();

void setSize(int width, int height);

void setClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void setClearDepth(float depth);
void clear(ClearFlags flags);

void setCullState(CullState cullState);

void setVertexArray(const VSInput* vertices);
void setIndexArray(const unsigned int* indices);

void drawArrays(size_t start, size_t count);
void drawElements(size_t start, size_t count);

void setVertexShader(IVertexShader* vs);
void setFragmentShader(IFragmentShader* fs);

void setRenderTarget(RenderTarget* renderTarget);

}  // namespace rs
