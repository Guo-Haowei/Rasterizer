#include "loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>
#include <algorithm>
#include <assimp/Importer.hpp>
#include "common/core_assert.h"

namespace rs {

using namespace gfx;

void loadTexture(Texture& texture, const char* path) {
    int width, height, channel;
    unsigned char* data = stbi_load(path, &width, &height, &channel, 0);
    if (data) {
        std::vector<Color> buffer(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int bufferIndex = y * width + x;
                Color& c = buffer[bufferIndex];
                int channelIndex = channel * bufferIndex;
                // NOTE: bitmap is in bgra format
                c.r = data[channelIndex + 2];
                c.g = data[channelIndex + 1];
                c.b = data[channelIndex + 0];
                c.a = channel == 4 ? data[channelIndex + 3] : 255;
            }
        }

        texture.create({ width, height, buffer.data() });
        stbi_image_free(data);
    } else {
        printf("Failed to load texture '%s'\n", path);
    }
}

Scene AssimpLoader::load(const char* path) {
    std::string full_path(path);
    printf("[Log] Loading scene '%s'\n", full_path.c_str());
    ::Assimp::Importer importer;
    const aiScene* aiscene = importer.ReadFile(full_path,
                                               aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_PopulateArmatureData);

    Scene ret;

    // check for errors
    if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode)  // if is Not Zero
    {
        printf("[ERROR] assimp failed to load:\n%s\n", importer.GetErrorString());
        return ret;
    }

    ret.materials.reserve(aiscene->mNumMaterials);
    for (uint32_t i = 0; i < aiscene->mNumMaterials; ++i) {
        aiMaterial* material = aiscene->mMaterials[i];
        ret.materials.emplace_back(processMaterial(aiscene->mMaterials[i], ""));
    }

    std::shared_ptr<Node> rootNode = processNode(aiscene->mRootNode, nullptr);

    ret.meshes.reserve(aiscene->mNumMeshes);
    for (uint32_t i = 0; i < aiscene->mNumMeshes; ++i) {
        ret.meshes.emplace_back(processMesh(aiscene->mMeshes[i]));
    }

    if (aiscene->mNumAnimations != 0) {
        ret.anim = processAnimation(aiscene->mAnimations[0]);
    }

    ret.root = std::move(rootNode);

    return ret;
}

Mesh AssimpLoader::processMesh(const aiMesh* aimesh) {
    Mesh mesh;
    mesh.name = aimesh->mName.C_Str();
    mesh.positions.reserve(aimesh->mNumVertices);
    for (uint32_t i = 0; i < aimesh->mNumVertices; ++i) {
        auto& position = aimesh->mVertices[i];
        mesh.positions.emplace_back(vec3(position.x, position.y, position.z));
        auto& normal = aimesh->mNormals[i];
        mesh.normals.emplace_back(vec3(normal.x, normal.y, normal.z));
        if (aimesh->mTextureCoords[0]) {
            auto& uv = aimesh->mTextureCoords[0][i];
            mesh.uvs.emplace_back(vec2(uv.x, uv.y));
        } else {
            mesh.uvs.emplace_back(vec2(0.0f));
        }
    }

    if (aimesh->HasBones()) {
        mesh.bones.reserve(aimesh->mNumBones);
        mesh.boneIds.resize(aimesh->mNumVertices);
        std::fill(mesh.boneIds.begin(), mesh.boneIds.end(), ivec4(0));
        mesh.weights.resize(aimesh->mNumVertices);
        std::fill(mesh.weights.begin(), mesh.weights.end(), vec4(0));
        for (uint32_t boneId = 0; boneId < aimesh->mNumBones; ++boneId) {
            aiBone* bone = aimesh->mBones[boneId];
            for (uint32_t j = 0; j < bone->mNumWeights; ++j) {
                const aiVertexWeight& vertexWeight = bone->mWeights[j];
                float* pWeight = &mesh.weights[vertexWeight.mVertexId].x;
                int* pBoneId = &mesh.boneIds[vertexWeight.mVertexId].x;
                size_t index = 0;
                for (; index < 4; ++index)
                    if (*(pWeight + index) == 0.0f)
                        break;

                *(pWeight + index) = vertexWeight.mWeight;
                *(pBoneId + index) = boneId;
            }
            std::string boneName = bone->mName.C_Str();
            auto it = m_nodeLookupTable.find(boneName);
            assert(it != m_nodeLookupTable.end());
            /// NOTE: could be broken here
            const auto& tmp = bone->mOffsetMatrix;
            mat4 offset(
                tmp.a1, tmp.b1, tmp.c1, tmp.d1,  //
                tmp.a2, tmp.b2, tmp.c2, tmp.d2,  //
                tmp.a3, tmp.b3, tmp.c3, tmp.d3,  //
                tmp.a4, tmp.b4, tmp.c4, tmp.d4   //
            );
            mesh.bones.push_back({ boneName, it->second, offset });
        }
    }

    // test
#if USING(DEV_BUILD)
    for (const auto& weight : mesh.weights) {
        float sum = weight.x + weight.y + weight.z + weight.w;
        static const float error = 0.0001f;
        assert(std::abs(sum - 1.0f) < error);
    }
#endif

    mesh.indices.reserve(aimesh->mNumFaces * 3);
    for (uint32_t i = 0; i < aimesh->mNumFaces; ++i) {
        aiFace& face = aimesh->mFaces[i];
        assert(face.mNumIndices == 3);
        mesh.indices.push_back(face.mIndices[0]);
        mesh.indices.push_back(face.mIndices[1]);
        mesh.indices.push_back(face.mIndices[2]);
    }

    mesh.matId = aimesh->mMaterialIndex;

    return mesh;
}

std::shared_ptr<Node> AssimpLoader::processNode(aiNode* ainode, std::shared_ptr<Node> parent) {
    std::shared_ptr<Node> node(new Node());
    if (parent != nullptr)
        node->parent = parent;

    node->name = ainode->mName.C_Str();
    if (ainode->mNumMeshes) {
        node->meshIdx = ainode->mMeshes[0];
    } else {
        node->meshIdx = -1;
    }

    m_nodeLookupTable.insert({ node->name, node.get() });

    const auto& tmp = ainode->mTransformation;
    node->transform = mat4(
        tmp.a1, tmp.b1, tmp.c1, tmp.d1,  //
        tmp.a2, tmp.b2, tmp.c2, tmp.d2,  //
        tmp.a3, tmp.b3, tmp.c3, tmp.d3,  //
        tmp.a4, tmp.b4, tmp.c4, tmp.d4   //
    );

    for (uint32_t childIndex = 0; childIndex < ainode->mNumChildren; ++childIndex) {
        aiNode* child = ainode->mChildren[childIndex];
        node->children.push_back(processNode(child, node));
    }

    return node;
}

Animation AssimpLoader::processAnimation(const aiAnimation* aianim) {
    Animation anim;
    double ticksPersecond = aianim->mTicksPerSecond;
    double duration = aianim->mDuration;
    anim.durationInseconds = duration / ticksPersecond;
    anim.nodeAnims.reserve(aianim->mNumChannels);
    for (uint32_t channelIndex = 0; channelIndex < aianim->mNumChannels; ++channelIndex) {
        aiNodeAnim* channel = aianim->mChannels[channelIndex];
        std::string nodeName = channel->mNodeName.C_Str();
        auto it = m_nodeLookupTable.find(nodeName);
        assert(it != m_nodeLookupTable.end());
        Node* node = it->second;

        NodeAnimation nodeAnim;
        nodeAnim.boneNode = node;
        assert(channel->mNumPositionKeys == channel->mNumRotationKeys);
        assert(channel->mNumRotationKeys == channel->mNumScalingKeys);
        for (unsigned int frameIndex = 0; frameIndex < channel->mNumPositionKeys; ++frameIndex) {
            const auto& position = channel->mPositionKeys[frameIndex];
            const auto& scale = channel->mScalingKeys[frameIndex];
            const auto& rotation = channel->mRotationKeys[frameIndex];
            const double positionTime = position.mTime;
#if USING(DEV_BUILD)
            const double scaleTime = scale.mTime;
            const double rotationTime = rotation.mTime;
            assert(positionTime == scaleTime);
            assert(positionTime == rotationTime);
#endif
            nodeAnim.keyframes.push_back({ vec3(position.mValue.x, position.mValue.y, position.mValue.z),
                                           vec3(scale.mValue.x, scale.mValue.y, scale.mValue.z),
                                           quat(rotation.mValue.w, rotation.mValue.x, rotation.mValue.y, rotation.mValue.z),
                                           positionTime / ticksPersecond });
        }
        anim.nodeAnims.push_back(nodeAnim);
    }

    return anim;
}

Material AssimpLoader::processMaterial(aiMaterial* material, const std::string& dir) {
    Material mat;
    mat.name = material->GetName().C_Str();
    // printf("Loading '%s'\n", mat.name.c_str());
    aiString path;
    if (material->GetTexture(aiTextureType_AMBIENT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
        // printf("Albedo map found: %s\n", path.C_Str());
        mat.diffuse = path.C_Str();
    }

    return mat;
}

}  // namespace rs
