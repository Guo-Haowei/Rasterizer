#include "example_base.h"

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
        vs_output.worldPosition = input.position;
        return vs_output;
    }

   private:
    static const unsigned int sVaryingFlags = VARYING_WORLD_POSITION;
};

class TriangleFS : public IFragmentShader {
    virtual Color processFragment(const VSOutput& input) override {
        Color color;
        vec4 position = .5f * input.worldPosition + .5f;
        color.r = static_cast<unsigned char>(255.f * position.r);
        color.g = static_cast<unsigned char>(255.f * position.g);
        color.b = static_cast<unsigned char>(255.f * position.b);
        color.a = 255;
        return color;
    }
};

class TriangleApp : public ExampleBase {
   public:
    TriangleApp();

    virtual void postInit() override;
    virtual void update(double deltaTime) override;

   private:
    TriangleVS m_vs;
    TriangleFS m_fs;
};

TriangleApp::TriangleApp()
    : ExampleBase(g_config) {
}

void TriangleApp::postInit() {
    rs::setVertexShader(&m_vs);
    rs::setFragmentShader(&m_fs);

    g_vertices[0].position = { +0.0f, +0.5f, 0.0f, 1.0f };
    g_vertices[1].position = { -0.5f, -0.5f, 0.0f, 1.0f };
    g_vertices[2].position = { +0.5f, -0.5f, 0.0f, 1.0f };

    rs::setVertexArray(g_vertices);
    rs::setCullState(BACK_FACE);
}

void TriangleApp::update(double deltaTime) {
    rs::clear(COLOR_BUFFER_BIT);
    rs::drawArrays(0, 3);
}

ExampleBase* g_pExample = new TriangleApp();
Config g_config = { 900, 540, "Triangle" };
