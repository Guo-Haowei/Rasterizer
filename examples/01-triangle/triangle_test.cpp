#include "example_base.h"
#include "rasterizer/ShaderProgram.h"

using namespace gfx;
using namespace rs;

VSInput g_vertices[3];

class TriangleVS : public IVertexShader {
   public:
    TriangleVS()
        : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput& input) override {
        VSOutput vs_output;
        vs_output.position = input.position;
        vs_output.color = input.color;
        return vs_output;
    }

   private:
    static const unsigned int sVaryingFlags = VARYING_COLOR;
};

class TriangleFS : public IFragmentShader {
    virtual Color processFragment(const VSOutput& input) override {
        Color color;
        color.r = static_cast<unsigned char>(255.f * input.color.r);
        color.g = static_cast<unsigned char>(255.f * input.color.g);
        color.b = static_cast<unsigned char>(255.f * input.color.b);
        color.a = 255;
        return color;
    }
};

class TriangleApp : public ExampleBase {
   public:
    TriangleApp();

    virtual void initialize() override;

    virtual void update(double deltaTime) override;

    virtual void finalize() override;

   private:
    TriangleVS m_vs;
    TriangleFS m_fs;
};

TriangleApp::TriangleApp()
    : ExampleBase({ 900, 540, "Triangle" }) {
}

void TriangleApp::initialize() {
    m_renderer.setSize(m_width, m_height);
    m_renderer.setVertexShader(&m_vs);
    m_renderer.setFragmentShader(&m_fs);

    g_vertices[0].position = { +0.0f, +0.5f, 0.0f, 1.0f };
    g_vertices[1].position = { -0.5f, -0.5f, 0.0f, 1.0f };
    g_vertices[2].position = { +0.5f, -0.5f, 0.0f, 1.0f };
    g_vertices[0].color = { 1.0f, 0.0f, 0.0f };
    g_vertices[1].color = { 0.0f, 1.0f, 0.0f };
    g_vertices[2].color = { 0.0f, 0.0f, 1.0f };
    m_renderer.setVertexArray(g_vertices);

    m_renderer.setCullState(Renderer::BACK_FACE);
}

void TriangleApp::update(double deltaTime) {
    m_renderer.clear(Renderer::COLOR_BUFFER_BIT);
    m_renderer.drawArrays(0, 3);
}

void TriangleApp::finalize() {
}

ExampleBase* g_pExample = new TriangleApp();
