#include <assert.h>
#include <cmath>
#include "../loader/loader.h"
#include "application.h"
#include "example_base.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

#define MODEL_PATH(NAME) ASSET_DIR NAME "/" NAME ".obj"

using namespace rs;
using namespace gfx;

class AnimatedModelVS : public IVertexShader {
   public:
    AnimatedModelVS()
        : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput &input) override {
        VSOutput vs_output;
        vs_output.worldPosition = M * input.position;
        vs_output.position = PV * (vs_output.worldPosition);
        vs_output.normal = M * input.normal;
        //vs_output.uv = input.uv;
        return vs_output;
    }

   public:
    gfx::mat4 PV;
    gfx::mat4 M = mat4(1);

   private:
    static const unsigned int sVaryingFlags =
        VARYING_NORMAL | VARYING_UV | VARYING_WORLD_POSITION;
};

class TexturePhongFS : public IFragmentShader {
   public:
    virtual gfx::Color processFragment(const VSOutput &input) override {
        const gfx::vec3 normal = gfx::normalize(gfx::vec3(input.normal));
        const gfx::vec3 color = .5f * (normal + 1.f);
        gfx::Color finalColor;
        finalColor.r = static_cast<unsigned char>(255 * color.r);
        finalColor.g = static_cast<unsigned char>(255 * color.g);
        finalColor.b = static_cast<unsigned char>(255 * color.b);
        finalColor.a = 255;
        return finalColor;
    }

   public:
};

struct MeshData {
    std::vector<VSInput> vertices;
    std::vector<unsigned int> indices;
};

class SponzaTest : public ExampleBase {
   public:
    SponzaTest();

    virtual void postInit() override;
    virtual void update(double deltaTime) override;

   private:
    void updatePV();
    void drawNode(const Node *node);

    AnimatedModelVS m_vs;
    TexturePhongFS m_fs;

    Texture m_texture;
    Scene m_scene;
    std::vector<MeshData> m_meshes;

    gfx::mat4 m_P;
    gfx::mat4 m_V;
    gfx::mat4 m_PV;
    gfx::vec3 m_eye = { 0, 1, 0 };
    float m_angle = 90.0f;
};

SponzaTest::SponzaTest()
    : ExampleBase(g_config) {}

void SponzaTest::updatePV() {
    constexpr float fovy = 0.785398f;  // 45.0 degree
    const float rad = glm::radians(m_angle);
    gfx::vec3 front = gfx::vec3(glm::sin(rad), 0.0f, glm::cos(rad));
    m_V = lookAt(m_eye, m_eye + front, vec3(0, 1, 0));
    m_P = perspectiveRH_NO(fovy, (float)m_width / (float)m_height, 1.0f, 40.0f);
    m_PV = m_P * m_V;
    m_vs.PV = m_PV;
}

void SponzaTest::postInit() {
    rs::setSize(m_width, m_height);
    rs::setVertexShader(&m_vs);
    rs::setFragmentShader(&m_fs);

    updatePV();

    // load model
    AssimpLoader loader;
    m_scene = loader.load(MODEL_PATH("sponza"));
    for (const Mesh &mesh : m_scene.meshes) {
        MeshData data;
        data.indices = mesh.indices;
        for (size_t i = 0; i < mesh.positions.size(); ++i) {
            VSInput input;
            input.position = vec4(mesh.positions[i], 1.0f);
            input.normal = vec4(mesh.normals[i], 0.0f);
            input.uv = mesh.uvs[i];
            data.vertices.push_back(input);
        }
        m_meshes.emplace_back(data);
    }

    rs::setCullState(BACK_FACE);
    m_scene.root->calWorldTransform();
}

void SponzaTest::update(double deltaTime) {
    constexpr float distance = 10.f;
    if (int horizontal = app::getKey(KeyCode::KEY_RIGHT) - app::getKey(KeyCode::KEY_LEFT)) {
        constexpr float angle = 1.f;
        m_angle += horizontal * angle;
        updatePV();
    } else if (int dz = app::getKey(KeyCode::KEY_W) - app::getKey(KeyCode::KEY_S)) {
        const float rad = glm::radians(m_angle);
        gfx::vec3 front = gfx::vec3(glm::sin(rad), 0.0f, glm::cos(rad));
        m_eye += dz * distance * front * float(deltaTime);
        updatePV();
    } else if (int dy = app::getKey(KeyCode::KEY_E) - app::getKey(KeyCode::KEY_Q)) {
        // const float rad = glm::radians(m_angle);
        // gfx::vec3 front = gfx::vec3(glm::cos(rad), 0.0f, glm::sin(rad));
        m_eye.y += dy * distance * float(deltaTime);
        updatePV();
    }

    rs::clear(COLOR_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < m_meshes.size(); ++i) {
        const MeshData &mesh = m_meshes[i];
        rs::setVertexArray(mesh.vertices.data());
        rs::setIndexArray(mesh.indices.data());
        rs::drawElements(0, mesh.indices.size());
    }
}

ExampleBase *g_pExample = new SponzaTest();
Config g_config = { 900, 540, "Sponza" };
