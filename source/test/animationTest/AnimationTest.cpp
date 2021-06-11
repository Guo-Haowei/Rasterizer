#include <assert.h>
#include <cmath>
#include "shaders/AnimatedModelVS.h"
#include "shaders/TexturePhongFS.h"
#include "utilities/Loader.h"
#include "viewer/BaseApp.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

#define MODEL_PATH(NAME)    ASSET_DIR "glTF/" NAME "/" NAME ".gltf"
#define MODEL_TEXTURE(NAME) ASSET_DIR "glTF/" NAME "/" NAME "_img0.jpg"

namespace hguo {

using namespace gfx;

class AnimationTest : public hguo::BaseApp {
   public:
    AnimationTest();

    virtual void initialize() override;

    virtual void update(double deltaTime) override;

    virtual void finalize() override;

   private:
    AnimatedModelVS m_vs;
    TexturePhongFS m_fs;

    Texture m_texture;
    Scene m_scene;
    std::vector<VSInput> m_vertices;
    std::vector<unsigned int> m_indices;

    gfx::mat4 m_P;
    gfx::mat4 m_V;
    gfx::mat4 m_PV;
    double m_elapsedTime = 0;
};

AnimationTest::AnimationTest()
    : BaseApp({ 900, 540, "Animation" }) {
}

void AnimationTest::initialize() {
    m_renderer.setSize(m_width, m_height);
    m_renderer.setVertexShader(&m_vs);
    m_renderer.setFragmentShader(&m_fs);

    // constant buffer
    const float fovy = 0.785398f;  // 45.0 degree
    m_V = lookAt(vec3(0, 2, 3), vec3(0, 1, 0), vec3(0, 1, 0));
    m_P = perspectiveRH_NO(fovy, (float)m_width / (float)m_height, 0.1f, 20.0f);
    m_PV = m_P * m_V;
    m_vs.M = mat4(1);
    m_vs.PV = m_PV;

    // load model
    AssimpLoader loader;
    m_scene = loader.load(MODEL_PATH("CesiumMan"));
    const Mesh& mesh = m_scene.meshes.at(0);
    m_indices = mesh.indices;
    for (size_t i = 0; i < mesh.positions.size(); ++i) {
        VSInput input;
        input.position = vec4(mesh.positions[i], 1.0f);
        input.normal = vec4(mesh.normals[i], 0.0f);
        input.uv = mesh.uvs[i];
        input.weights = mesh.weights[i];
        input.boneId = mesh.boneIds[i];
        m_vertices.push_back(input);
    }

    loadTexture(m_texture, MODEL_TEXTURE("CesiumMan"));
    m_fs.m_texture = &m_texture;

    m_renderer.setVertexArray(m_vertices.data());
    m_renderer.setIndexArray(m_indices.data());
    m_renderer.setCullState(Renderer::BACK_FACE);
}

static void findAnimMatrix(const NodeAnimation& anim, double time, mat4& localTransform) {
    size_t endFrameIndex = 0;
    const size_t frameCount(anim.keyframes.size());
    for (; endFrameIndex < frameCount; ++endFrameIndex) {
        const Keyframe& endFrame = anim.keyframes[endFrameIndex];
        if (time <= endFrame.timeInSeconds) {
            size_t beginFrameIndex = (endFrameIndex - 1 + frameCount) % frameCount;
            const Keyframe& beginFrame = anim.keyframes[beginFrameIndex];
            // interpolate between two frames
            float t = static_cast<float>((time - beginFrame.timeInSeconds) / (endFrame.timeInSeconds - beginFrame.timeInSeconds));
            vec3 p = mix(beginFrame.position, endFrame.position, t);
            vec3 s = mix(beginFrame.scale, endFrame.scale, t);
            quat r = mix(beginFrame.rotation, endFrame.rotation, t);

            mat4 T = translate(mat4(1), p);
            mat4 S = scale(mat4(1), s);
            mat4 R = toMat4(r);
            localTransform = T * R * S;
            return;
        }
    }

    ASSERT(0);
}

void AnimationTest::update(double deltaTime) {
    // time
    const Animation& anim = m_scene.anim;
    double time = std::fmod(m_elapsedTime, anim.durationInseconds);
    ASSERT(time <= anim.durationInseconds);
    ASSERT(time >= 0.0);

    // int rotate = (int)ImGui::IsKeyDown(GLFW_KEY_RIGHT) - (int)ImGui::IsKeyDown(GLFW_KEY_LEFT);

    // FIXME: dummy elapsed time
    const mat4 R = rotate(mat4(1), 0.003f, vec3(0, 1, 0));
    m_vs.M = R * m_vs.M;

    // update animation
    for (auto& nodeAnim : m_scene.anim.nodeAnims) {
        findAnimMatrix(nodeAnim, time, nodeAnim.boneNode->transform);
    }

    m_scene.root->calWorldTransform();

    const auto& bones = m_scene.meshes[0].bones;
    m_vs.boneMatrices.resize(bones.size());
    for (size_t i = 0; i < bones.size(); ++i) {
        const Bone& bone = bones[i];
        const mat4 boneMatrix = bone.boneNode->worldTransform * bone.offsetMatrix;
        m_vs.boneMatrices[i] = boneMatrix;
    }

    m_renderer.clear(Renderer::COLOR_DEPTH_BUFFER_BIT);
    m_renderer.drawElements(0, m_indices.size());

    m_elapsedTime += deltaTime;
}

void AnimationTest::finalize() {
}

BaseApp* g_pApp = new AnimationTest();

}  // namespace hguo
