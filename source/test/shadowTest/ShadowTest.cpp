#if 0
#include <GLFW/glfw3.h>
#include <assert.h>
#include <cmath>
#include "imgui.h"
#include "shaders/AnimatedModelDepthVS.h"
#include "shaders/AnimatedModelVS.h"
#include "shaders/DepthFS.h"
#include "shaders/ModelDepthVS.h"
#include "shaders/ModelVS.h"
#include "shaders/PhongWithShadowFS.h"
#include "shaders/TexturePhongFS.h"
#include "utilities/Loader.h"
#include "viewer/BaseApp.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

#define SHADOW_MAP_RES 1024

#define MODEL_PATH(NAME) ASSET_DIR "glTF/" NAME "/" NAME ".gltf"
#define MODEL_TEXTURE(NAME) ASSET_DIR "glTF/" NAME "/" NAME "_img0.jpg"

namespace hguo {

struct MeshObject {
    std::vector<VSInput> vertices;
    std::vector<unsigned int> indices;
};

class ShadowTest : public hguo::BaseApp {
   public:
    ShadowTest();

    virtual void initialize() override;

    virtual void update(double deltaTime) override;

    virtual void finalize() override;

    void drawMeshObject(const MeshObject& meshObject);

    virtual const Texture& getTexture() const override;

   private:
    AnimatedModelVS m_modelVs;
    TexturePhongFS m_modelFs;
    ModelVS m_floorVs;
    PhongWithShadowFS m_floorFs;

    // depth
    AnimatedModelDepthVS m_modelDepthVs;
    ModelDepthVS m_floorDepthVs;
    DepthFS m_depthFs;

    Texture m_texture;
    Scene m_scene;

    MeshObject m_model;
    MeshObject m_floor;

    Matrix4 m_cameraPV;
    Matrix4 m_lightPV;

    RenderTarget m_depthTarget;

    Vector3 m_lightPosition { 3 };
    double m_elapsedTime = 0;

    bool m_showDepthBuffer = false;
};

ShadowTest::ShadowTest() : BaseApp({ 900, 540, "Shadow" }) {
    m_depthTarget.create({ SHADOW_MAP_RES, SHADOW_MAP_RES, true, true });
}

const Texture& ShadowTest::getTexture() const {
    if (m_showDepthBuffer)
        return m_depthTarget.getColorBuffer();
    else
        return m_renderTarget.getColorBuffer();
}

void ShadowTest::drawMeshObject(const MeshObject& meshObject) {
    m_renderer.setIndexArray(meshObject.indices.data());
    m_renderer.setVertexArray(meshObject.vertices.data());
    m_renderer.drawElements(0, meshObject.indices.size());
}

void ShadowTest::initialize() {
    m_renderer.setSize(m_width, m_height);

    // load model
    AssimpLoader loader;
    m_scene = loader.load(MODEL_PATH("CesiumMan"));
    const Mesh& mesh = m_scene.meshes.at(0);
    m_model.indices = mesh.indices;
    for (size_t i = 0; i < mesh.positions.size(); ++i) {
        VSInput input;
        input.position = Vector4(mesh.positions[i], 1.0f);
        input.normal = Vector4(mesh.normals[i], 0.0f);
        input.uv = mesh.uvs[i];
        input.weights = mesh.weights[i];
        input.boneId = mesh.boneIds[i];
        m_model.vertices.push_back(input);
    }

    loadTexture(m_texture, MODEL_TEXTURE("CesiumMan"));
    m_modelFs.m_texture = &m_texture;
    m_floorFs.m_depthBuffer = &m_depthTarget.m_depthBuffer;

    // floor
    {
        static const float size = 2.5f;
        static const float y = 0.0f;
        Vector3 vertices[4] = {
            { -size, y, +size },  // top left
            { -size, y, -size },  // bottom left
            { +size, y, -size },  // bottom right
            { +size, y, +size }   // top right
        };

        std::vector<unsigned int> indices = {
            0, 2, 1,
            0, 3, 2
        };

        for (size_t i = 0; i < 4; ++i) {
            VSInput input;
            input.position = Vector4(vertices[i], 1.0f);
            input.normal = Vector4(0, 1, 0, 0);
            m_floor.vertices.push_back(input);
        }

        m_floor.indices = indices;
    }

    // model shader uniforms
    // aspect ratio
    float aspect = (float)m_width / m_height;
    float rad45degree = 0.785398f;
    float rad30degree = 0.523599f;
    const Matrix4 R = rotateY(rad45degree);
    const Matrix4 cameraV = lookAt(Vector3(0, 2, 3), Vector3::UnitY, Vector3::UnitY);
    const Matrix4 cameraP = perspectiveRH_NO(rad45degree, aspect, 1.0f, 10.0f);

    const Matrix4 lightP = perspectiveRH_NO(rad30degree, 1.0f, 1.0f, 10.0f);
    const Matrix4 lightV = lookAt(m_lightPosition, Vector3::Zero, Vector3::UnitY);
    const Matrix4 lightPV = lightP * lightV;

    m_cameraPV = cameraP * cameraV;
    m_modelVs.PV = m_cameraPV;
    m_modelVs.M = R;
    m_floorVs.lightPV = lightPV;
    m_floorVs.PV = m_cameraPV;
    m_floorVs.M = Matrix4::Identity;

    m_lightPV = lightP * lightV;
    m_modelDepthVs.PV = m_lightPV;
    m_modelDepthVs.M = m_modelVs.M;
    m_floorDepthVs.PV = m_lightPV;
    m_floorDepthVs.M = m_floorVs.M;
    m_floorFs.m_lightPosition = m_lightPosition;
    m_modelFs.m_lightPosition = m_lightPosition;

    auto boneCount = m_scene.meshes[0].bones.size();
    m_modelVs.boneMatrices.resize(boneCount);
    m_modelDepthVs.boneMatrices.resize(boneCount);

    m_renderer.setCullState(Renderer::BACK_FACE);
}

static void findAnimMatrix(const NodeAnimation& anim, double time, Matrix4& localTransform) {
    size_t endFrameIndex = 0;
    const size_t frameCount = anim.keyframes.size();
    for (; endFrameIndex < frameCount; ++endFrameIndex) {
        const Keyframe& endFrame = anim.keyframes[endFrameIndex];
        if (time <= endFrame.timeInSeconds) {
            size_t beginFrameIndex = (endFrameIndex - 1 + frameCount) % frameCount;
            const Keyframe& beginFrame = anim.keyframes[beginFrameIndex];
            // interpolate between two frames
            float t = static_cast<float>((time - beginFrame.timeInSeconds) / (endFrame.timeInSeconds - beginFrame.timeInSeconds));
            Vector3 position = three::lerp(beginFrame.position, endFrame.position, t);
            Vector3 scale = three::lerp(beginFrame.scale, endFrame.scale, t);
            Quaternion quat = three::slerp(beginFrame.rotation, endFrame.rotation, t);

            Matrix4 T = three::translate(position);
            Matrix4 S = three::scale(scale);
            Matrix4 R = three::rotate(quat);
            localTransform = T * R * S;
            return;
        }
    }

    ASSERT(0);
}

void ShadowTest::update(double deltaTime) {
    if (ImGui::IsKeyPressed(GLFW_KEY_D)) {
        m_showDepthBuffer = !m_showDepthBuffer;
    }

    // time
    const Animation& anim = m_scene.anim;
    double time = std::fmod(m_elapsedTime, anim.durationInseconds);
    ASSERT(time <= anim.durationInseconds);
    ASSERT(time >= 0.0);

    // update animation
    for (auto& nodeAnim : m_scene.anim.nodeAnims) {
        findAnimMatrix(nodeAnim, time, nodeAnim.boneNode->transform);
    }

    m_scene.root->calWorldTransform();

    const auto& bones = m_scene.meshes[0].bones;
    for (size_t i = 0; i < bones.size(); ++i) {
        const Bone& bone = bones[i];
        m_modelVs.boneMatrices[i] = bone.boneNode->worldTransform * bone.offsetMatrix;
        m_modelDepthVs.boneMatrices[i] = m_modelVs.boneMatrices[i];
    }

    // depth pass
    {
        // m_renderer.setClearColor(255, 255, 255, 255);
        m_renderer.setRenderTarget(&m_depthTarget);
        m_renderer.clear(Renderer::COLOR_DEPTH_BUFFER_BIT);

        m_renderer.setVertexShader(&m_modelDepthVs);
        m_renderer.setFragmentShader(&m_depthFs);
        drawMeshObject(m_model);

        m_renderer.setVertexShader(&m_floorDepthVs);
        m_renderer.setFragmentShader(&m_depthFs);
        drawMeshObject(m_floor);
    }

    // color pass
    {
        m_renderer.setRenderTarget(&m_renderTarget);
        m_renderer.clear(Renderer::COLOR_DEPTH_BUFFER_BIT);

        m_renderer.setVertexShader(&m_modelVs);
        m_renderer.setFragmentShader(&m_modelFs);
        drawMeshObject(m_model);

        m_renderer.setVertexShader(&m_floorVs);
        m_renderer.setFragmentShader(&m_floorFs);
        drawMeshObject(m_floor);
    }

    m_elapsedTime += deltaTime;
}

void ShadowTest::finalize() {
}

BaseApp* g_pApp = new ShadowTest();

}  // namespace hguo
#endif
