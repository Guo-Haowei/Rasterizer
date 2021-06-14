#include <assert.h>
#include <cmath>
#include "../loader/loader.h"
#include "application.h"
#include "example_base.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

#define MODEL_PATH(NAME)    ASSET_DIR NAME "/" NAME ".gltf"
#define MODEL_TEXTURE(NAME) ASSET_DIR NAME "/" NAME "_img0.jpg"

using namespace rs;
using namespace gfx;

class AnimatedModelVS : public IVertexShader {
   public:
    AnimatedModelVS()
        : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput &input) override {
        VSOutput vs_output;
        gfx::mat4 boneTransform = boneMatrices[input.boneId.x] * input.weights.x;
        boneTransform += (boneMatrices[input.boneId.y] * input.weights.y);
        boneTransform += (boneMatrices[input.boneId.z] * input.weights.z);
        boneTransform += (boneMatrices[input.boneId.w] * input.weights.w);
        gfx::mat4 worldTransform = M * boneTransform;
        vs_output.worldPosition = worldTransform * input.position;
        vs_output.position = PV * (vs_output.worldPosition);
        vs_output.normal = worldTransform * input.normal;
        vs_output.uv = input.uv;
        return vs_output;
    }

   public:
    gfx::mat4 PV;
    gfx::mat4 M = mat4(1);
    std::vector<gfx::mat4> boneMatrices;

   private:
    static const unsigned int sVaryingFlags =
        VARYING_NORMAL | VARYING_UV | VARYING_WORLD_POSITION;
};

class TexturePhongFS : public IFragmentShader {
   public:
    virtual gfx::Color processFragment(const VSOutput &input) override {
        const gfx::vec3 normal = gfx::normalize(gfx::vec3(input.normal));
        const gfx::vec3 worldPosition(input.worldPosition);
        const gfx::vec3 lightDirection =
            gfx::normalize(m_lightPosition - worldPosition);
        float dot = gfx::dot(lightDirection, normal);
        float diffusePower = gfx::max(dot, 0.0f);
        gfx::Color textureColor = m_texture->sample(input.uv);
        gfx::Color finalColor;
        finalColor.r = static_cast<unsigned char>(diffusePower * textureColor.r);
        finalColor.g = static_cast<unsigned char>(diffusePower * textureColor.g);
        finalColor.b = static_cast<unsigned char>(diffusePower * textureColor.b);
        finalColor.a = 255;
        return finalColor;
    }

   public:
    gfx::vec3 m_lightPosition { 0, 3, 3 };
    const Texture *m_texture = nullptr;
};

class AnimationTest : public ExampleBase {
   public:
    AnimationTest();

    virtual void initialize() override;

    virtual void update(double deltaTime) override;

    virtual void finalize() override;

   private:
    void updatePV();

    AnimatedModelVS m_vs;
    TexturePhongFS m_fs;

    Texture m_texture;
    Scene m_scene;
    std::vector<VSInput> m_vertices;
    std::vector<unsigned int> m_indices;

    gfx::mat4 m_P;
    gfx::mat4 m_V;
    gfx::mat4 m_PV;
    gfx::vec3 m_eye = { 0, 1, 3 };
    double m_elapsedTime = 0;
};

AnimationTest::AnimationTest()
    : ExampleBase(g_config) {}

void AnimationTest::updatePV() {
    constexpr float fovy = 0.785398f;  // 45.0 degree
    m_V = lookAt(m_eye, m_eye - vec3(0, 0, 1), vec3(0, 1, 0));
    m_P = perspectiveRH_NO(fovy, (float)m_width / (float)m_height, 0.1f, 20.0f);
    m_PV = m_P * m_V;
    m_vs.PV = m_PV;
}

void AnimationTest::initialize() {
    m_renderer.setSize(m_width, m_height);
    m_renderer.setVertexShader(&m_vs);
    m_renderer.setFragmentShader(&m_fs);

    updatePV();

    // load model
    AssimpLoader loader;
    m_scene = loader.load(MODEL_PATH("CesiumMan"));
    const Mesh &mesh = m_scene.meshes.at(0);
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

static void findAnimMatrix(const NodeAnimation &anim, double time,
                           mat4 &localTransform) {
    size_t endFrameIndex = 0;
    const size_t frameCount(anim.keyframes.size());
    for (; endFrameIndex < frameCount; ++endFrameIndex) {
        const Keyframe &endFrame = anim.keyframes[endFrameIndex];
        if (time <= endFrame.timeInSeconds) {
            size_t beginFrameIndex = (endFrameIndex - 1 + frameCount) % frameCount;
            const Keyframe &beginFrame = anim.keyframes[beginFrameIndex];
            // interpolate between two frames
            float t = static_cast<float>(
                (time - beginFrame.timeInSeconds) /
                (endFrame.timeInSeconds - beginFrame.timeInSeconds));
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
    const Animation &anim = m_scene.anim;
    double time = std::fmod(m_elapsedTime, anim.durationInseconds);
    time = time == time ? time : 0.0;
    ASSERT(time <= anim.durationInseconds);
    ASSERT(time >= 0.0f);

    if (int r = app::getKey(KeyCode::KEY_RIGHT) - app::getKey(KeyCode::KEY_LEFT)) {
        constexpr float rotateFactor = 1.5f;
        float angle = float(deltaTime);
        angle = r > 0 ? angle : -angle;
        const mat4 R = rotate(mat4(1), rotateFactor * angle, vec3(0, 1, 0));
        m_vs.M = R * m_vs.M;
    }

    if (int zoom = app::getKey(KeyCode::KEY_DOWN) - app::getKey(KeyCode::KEY_UP)) {
        constexpr float distance = 1.f;
        m_eye.z += zoom * distance * float(deltaTime);
        updatePV();
    }

    // update animation
    for (auto &nodeAnim : m_scene.anim.nodeAnims) {
        findAnimMatrix(nodeAnim, time, nodeAnim.boneNode->transform);
    }

    m_scene.root->calWorldTransform();

    const auto &bones = m_scene.meshes[0].bones;
    m_vs.boneMatrices.resize(bones.size());
    for (size_t i = 0; i < bones.size(); ++i) {
        const Bone &bone = bones[i];
        const mat4 boneMatrix = bone.boneNode->worldTransform * bone.offsetMatrix;
        m_vs.boneMatrices[i] = boneMatrix;
    }

    m_renderer.clear(Renderer::COLOR_DEPTH_BUFFER_BIT);
    m_renderer.drawElements(0, m_indices.size());

    m_elapsedTime += deltaTime;
}

void AnimationTest::finalize() {}

ExampleBase *g_pExample = new AnimationTest();
Config g_config = { 900, 540, "Animation" };
