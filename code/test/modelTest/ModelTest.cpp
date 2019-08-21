#include "shaders/PhongFS.h"
#include "shaders/ModelVS.h"
#include "viewer/BaseApp.h"
#include "utilities/Loader.h"
#include "imgui.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

#define MODEL_PATH(NAME) ASSET_DIR "/glTF-Sample-Models/2.0/" NAME "/glTF/" NAME ".gltf"
#define MODEL_TEXTURE(NAME) ASSET_DIR "/glTF-Sample-Models/2.0/" NAME "/glTF/" NAME "_img0.jpg"

namespace hguo {

class ModelTest : public hguo::BaseApp
{
public:
    ModelTest();

    virtual void initialize() override;

    virtual void update(double deltaTime) override;

    virtual void finalize() override;

    virtual void gui() override;

private:
    ModelVS m_vs;
    PhongFS m_fs;

    Texture m_texture;
    Scene m_scene;
    std::vector<VSInput> m_vertices;
    std::vector<unsigned int> m_indices;

    Matrix4 m_P;
    Matrix4 m_V;
    Matrix4 m_PV;
    Matrix4 m_M;
};

ModelTest::ModelTest() : BaseApp({ 900, 540, "Model"})
{
}

void ModelTest::gui()
{
    ImGui::Begin("Debug");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
    ImGui::Begin("Material");
    ImGui::Text("Shininess %d", m_fs.shininess);
    if (ImGui::Button("Increase"))
    {
        m_fs.shininess = std::min(m_fs.shininess * 2, 512);
    }
    if (ImGui::Button("Decrease"))
    {
        m_fs.shininess = std::max(m_fs.shininess / 2, 1);
    }
    ImGui::End();
}

void ModelTest::initialize()
{
    m_renderer.setSize(m_width, m_height);
    m_renderer.setVertexShader(&m_vs);
    m_renderer.setFragmentShader(&m_fs);

    // constant buffer
    const Vector3 cameraPostition(0, 2, 8);

    const float fovy = 0.785398f; // 45.0 degree
    m_V = lookAt(cameraPostition, Vector3::UnitY, Vector3::UnitY);
    m_P = perspectiveRH_NO(fovy, (float)m_width / (float)m_height, 0.1f, 20.0f);
    m_PV = m_P * m_V;
    m_M = Matrix4::Identity;
    m_vs.M = m_M;
    m_vs.PV = m_PV;
    m_fs.cameraPosition = cameraPostition;
    m_fs.m_lightPosition = cameraPostition;
    m_fs.shininess = 8;

    // load model
    AssimpLoader loader;
    m_scene = loader.load(ASSET_DIR "objs/teapot.obj");
    const Mesh& mesh = m_scene.meshes.at(0);
    m_indices = mesh.indices;
    for (size_t i = 0; i < mesh.positions.size(); ++i)
    {
        VSInput input;
        input.position = Vector4(mesh.positions[i], 1.0f);
        input.normal = Vector4(mesh.normals[i], 0.0f);
        if (mesh.uvs.size())
            input.uv = mesh.uvs[i];
        if (mesh.weights.size())
            input.weights = mesh.weights[i];
        if (mesh.boneIds.size())
            input.boneId = mesh.boneIds[i];

        m_vertices.push_back(input);
    }

    m_renderer.setVertexArray(m_vertices.data());
    m_renderer.setIndexArray(m_indices.data());
    m_renderer.setCullState(Renderer::BACK_FACE);
}

void ModelTest::update(double deltaTime)
{
    m_renderer.clear(Renderer::COLOR_DEPTH_BUFFER_BIT);
    m_renderer.drawElements(0, m_indices.size());
}

void ModelTest::finalize()
{

}

BaseApp* g_pApp = new ModelTest();

} // namespace hguo
