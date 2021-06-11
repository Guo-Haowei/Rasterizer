#include "../loader/loader.h"
#include "example_base.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

using namespace gfx;
using namespace rs;

class TextureVs : public IVertexShader {
   public:
    TextureVs()
        : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput &input) override {
        VSOutput vs_output;
        vs_output.position = input.position;
        vs_output.uv = gfx::vec2(input.position);
        vs_output.uv += 0.5f;
        vs_output.uv.y = 1.0f - vs_output.uv.y;
        vs_output.position = PVM * vs_output.position;
        return vs_output;
    }

   public:
    gfx::mat4 PVM;

   private:
    static const unsigned int sVaryingFlags = VARYING_UV;
};

class TextureFs : public IFragmentShader {
   public:
    virtual gfx::Color processFragment(const VSOutput &input) override {
        gfx::Color color = m_cubeTexture->sample(input.uv);
        return color;
    }

   public:
    const Texture *m_cubeTexture;
};

VSInput g_vertices[4];

const unsigned int g_indices[6] = { 0, 1, 2, 0, 2, 3 };

class TextureTest : public ExampleBase {
   public:
    TextureTest();

    virtual void initialize() override;

    virtual void update(double deltaTime) override;

    virtual void finalize() override;

   private:
    TextureVs m_vs;
    TextureFs m_fs;

    Texture m_texture;

    mat4 P;
    mat4 V;
    mat4 PV;
};

TextureTest::TextureTest()
    : ExampleBase({ 900, 540, "Texture" }) {}

void TextureTest::initialize() {
    m_renderer.setSize(m_width, m_height);
    m_renderer.setVertexShader(&m_vs);
    m_renderer.setFragmentShader(&m_fs);

    // mesh
    g_vertices[0].position = { -0.5f, +0.5f, 0.0f, 1.0f };  // top left
    g_vertices[1].position = { -0.5f, -0.5f, 0.0f, 1.0f };  // bottom left
    g_vertices[2].position = { +0.5f, -0.5f, 0.0f, 1.0f };  // bottom right
    g_vertices[3].position = { +0.5f, +0.5f, 0.0f, 1.0f };  // top right

    m_renderer.setVertexArray(g_vertices);
    m_renderer.setIndexArray(g_indices);

    // constant buffer
    const float fovy = 0.785398f;  // 45.0 degree
    V = lookAt(vec3(0, -2, 2), vec3(0), vec3(0, 1, 0));
    P = perspectiveRH_NO(fovy, (float)m_width / (float)m_height, 0.1f, 10.0f);
    PV = P * V;

    // texture
    loadTexture(m_texture, ASSET_DIR "images/texture.jpg");
    m_fs.m_cubeTexture = &m_texture;
}

void TextureTest::update(double deltaTime) {
    static const mat4 M0 = translate(mat4(1), vec3(0.0f, 0.0f, 0.5f));
    static const mat4 M1 = translate(mat4(1), vec3(0.0f, 0.0f, -0.5f));
    m_renderer.clear(Renderer::COLOR_DEPTH_BUFFER_BIT);
    m_vs.PVM = PV * M0;
    m_renderer.drawElements(0, 6);
    m_vs.PVM = PV * mat4(1);
    m_renderer.drawElements(0, 6);
    m_vs.PVM = PV * M1;
    m_renderer.drawElements(0, 6);
}

void TextureTest::finalize() {}

ExampleBase *g_pExample = new TextureTest();
