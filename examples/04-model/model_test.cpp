#include <assert.h>
#include <cmath>
#include <unordered_map>
#include "../loader/loader.h"
#include "application.h"
#include "example_base.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

#define MODEL_NAME        "sponza"
#define MODEL_PATH(NAME)  ASSET_DIR NAME "/" NAME ".obj"
#define TEXTURE_DIR(NAME) ASSET_DIR NAME

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
    static const unsigned int sVaryingFlags = VARYING_NORMAL;
    // VARYING_NORMAL | VARYING_UV | VARYING_WORLD_POSITION;
};

class TexturePhongFS : public IFragmentShader {
   public:
    virtual gfx::Color processFragment(const VSOutput &input) override {
        constexpr gfx::vec3 lightPosition { 0, 5, 0 };
        gfx::vec3 normal = gfx::normalize(gfx::vec3(input.normal));
        normal = normal * .5f + .5f;
        gfx::Color finalColor;
        finalColor.r = static_cast<unsigned char>(normal.r * 255.f);
        finalColor.g = static_cast<unsigned char>(normal.g * 255.f);
        finalColor.b = static_cast<unsigned char>(normal.b * 255.f);
        finalColor.a = 255;
        return finalColor;
    }

   public:
    const Texture *m_texture = nullptr;
};

struct MeshData {
    std::vector<VSInput> vertices;
    std::vector<unsigned int> indices;
    const Texture *texture;
};

class ModelTest : public ExampleBase {
   public:
    ModelTest();

    virtual void postInit() override;
    virtual void update(double deltaTime) override;

   private:
    void updatePV();
    void drawNode(const Node *node);

    AnimatedModelVS m_vs;
    TexturePhongFS m_fs;

    Scene m_scene;
    std::vector<MeshData> m_meshes;

    gfx::mat4 m_P;
    gfx::mat4 m_V;
    gfx::mat4 m_PV;
    gfx::vec3 m_eye = { 0, 400, 0 };
    float m_angle = 90.0f;
    std::unordered_map<std::string, Texture> m_textures;
};

ModelTest::ModelTest()
    : ExampleBase(g_config) {}

void ModelTest::updatePV() {
    constexpr float fovy = 0.985398f;
    constexpr float near = 1.0f;
    constexpr float far = 5000.0f;
    const float rad = glm::radians(m_angle);
    gfx::vec3 front = gfx::vec3(glm::sin(rad), 0.0f, glm::cos(rad));
    m_V = lookAt(m_eye, m_eye + front, vec3(0, 1, 0));
    m_P = perspectiveRH_NO(fovy, (float)m_width / (float)m_height, near, far);
    m_PV = m_P * m_V;
    m_vs.PV = m_PV;
}

void ModelTest::postInit() {
    rs::setSize(m_width, m_height);
    rs::setVertexShader(&m_vs);
    rs::setFragmentShader(&m_fs);

    updatePV();

    // load model and materials
    AssimpLoader loader;
    m_scene = loader.load(MODEL_PATH(MODEL_NAME));
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

        const Material &mat = m_scene.materials.at(mesh.matId);
        auto it = m_textures.find(mat.name);
        if (it == m_textures.end()) {
            Texture texture;

            char buf[256];
            snprintf(buf, sizeof(buf) - 1, "%s/%s", TEXTURE_DIR(MODEL_NAME), mat.diffuse.c_str());
            loadTexture(texture, buf);
            m_textures[mat.name] = texture;
        }

        data.texture = &m_textures.at(mat.name);
        m_meshes.emplace_back(data);
    }

    rs::setCullState(BACK_FACE);
    m_scene.root->calWorldTransform();
}

void ModelTest::update(double deltaTime) {
    constexpr float distance = 100.f;
    if (int dx = app::getKey(KeyCode::KEY_A) - app::getKey(KeyCode::KEY_D)) {
        const float rad = glm::radians(m_angle + 90.0f);
        gfx::vec3 front = gfx::vec3(glm::sin(rad), 0.0f, glm::cos(rad));
        m_eye += dx * distance * front * float(deltaTime);
        updatePV();
    } else if (int dz = app::getKey(KeyCode::KEY_W) - app::getKey(KeyCode::KEY_S)) {
        const float rad = glm::radians(m_angle);
        gfx::vec3 front = gfx::vec3(glm::sin(rad), 0.0f, glm::cos(rad));
        m_eye += dz * distance * front * float(deltaTime);
        updatePV();
    } else if (int dy = app::getKey(KeyCode::KEY_E) - app::getKey(KeyCode::KEY_Q)) {
        m_eye.y += dy * distance * float(deltaTime);
        updatePV();
    } else if (int ry = app::getKey(KeyCode::KEY_LEFT) - app::getKey(KeyCode::KEY_RIGHT)) {
        m_angle += ry * float(deltaTime) * 30.f;
        updatePV();
    }

    rs::clear(COLOR_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < m_meshes.size(); ++i) {
        const MeshData &mesh = m_meshes[i];
        rs::setVertexArray(mesh.vertices.data());
        rs::setIndexArray(mesh.indices.data());
        m_fs.m_texture = mesh.texture;
        rs::drawElements(0, mesh.indices.size());
    }
}

ExampleBase *g_pExample = new ModelTest();
Config g_config = { 900, 540, "Model" };
