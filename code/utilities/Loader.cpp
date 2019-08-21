#include "Loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <assert.h>
#include <algorithm>

namespace hguo {

void loadTexture(Texture& texture, const char* path)
{
    int width, height, channel;
    unsigned char *data = stbi_load(path, &width, &height, &channel, 0);
    if (data)
    {
        std::vector<Color> buffer(width * height);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int bufferIndex = y * width + x;
                Color& c = buffer[bufferIndex];
                int channelIndex = channel * bufferIndex;
                c.r = data[channelIndex];
                c.g = data[channelIndex + 1];
                c.b = data[channelIndex + 2];
                c.a = channel == 4 ? data[channelIndex + 3] : 255;
            }
        }

        texture.create({width, height, buffer.data()});
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture " << path << std::endl;
    }
}

Scene AssimpLoader::load(const char* path)
{
    std::string full_path(path);
    std::cout << "[Log] Loading scene [" << full_path << "]" << std::endl;
    ::Assimp::Importer importer;
    const aiScene* aiscene = importer.ReadFile(full_path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PopulateArmatureData
    );

    Scene ret;

    // check for errors
    if(!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode) // if is Not Zero
    {
        std::cout << "[ERROR] assimp failed to load.";
        std::cout << importer.GetErrorString() << std::endl;
        return ret;
    }

    // m_scene->materials.reserve(scene->mNumMaterials);
    // for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    // {
    //     aiMaterial* material = scene->mMaterials[i];
    //     processMaterial(scene->mMaterials[i], dir);
    // }

    std::shared_ptr<Node> rootNode = processNode(aiscene->mRootNode, nullptr);

    ret.meshes.reserve(aiscene->mNumMeshes);
    for (uint32_t i = 0; i < aiscene->mNumMeshes; ++i)
    {
        processMesh(ret, aiscene->mMeshes[i]);
    }

    if (aiscene->mNumAnimations != 0)
    {
        processAnimation(ret, aiscene->mAnimations[0]);
    }

    ret.root = std::move(rootNode);

    return ret;
}

void AssimpLoader::processMesh(Scene& scene, const aiMesh* aimesh)
{
    scene.meshes.push_back({});
    Mesh& mesh = scene.meshes.back();

    mesh.name = aimesh->mName.C_Str();
    mesh.positions.reserve(aimesh->mNumVertices);
    for (uint32_t i = 0; i < aimesh->mNumVertices; ++i)
    {
        auto& position = aimesh->mVertices[i];
        mesh.positions.push_back(Vector3(position.x, position.y, position.z));
        auto& normal = aimesh->mNormals[i];
        mesh.normals.push_back(Vector3(normal.x, normal.y, normal.z));
        if (aimesh->mTextureCoords[0])
        {
            auto& uv = aimesh->mTextureCoords[0][i];
            mesh.uvs.push_back(Vector2(uv.x, uv.y));
        }
        else
        {
            mesh.uvs.push_back(Vector2(0.0f));
        }
    }

    if (aimesh->HasBones())
    {
        mesh.bones.reserve(aimesh->mNumBones);
        mesh.boneIds.resize(aimesh->mNumVertices);
        std::fill(mesh.boneIds.begin(), mesh.boneIds.end(), Vector4i::Zero);
        mesh.weights.resize(aimesh->mNumVertices);
        std::fill(mesh.weights.begin(), mesh.weights.end(), Vector4::Zero);
        for (uint32_t boneId = 0; boneId < aimesh->mNumBones; ++boneId)
        {
            aiBone* bone = aimesh->mBones[boneId];
            for (uint32_t j = 0; j < bone->mNumWeights; ++j)
            {
                const aiVertexWeight &vertexWeight = bone->mWeights[j];
                float *pWeight = &mesh.weights[vertexWeight.mVertexId].x;
                int *pBoneId = &mesh.boneIds[vertexWeight.mVertexId].x;
                size_t index = 0;
                for (; index < 4; ++index)
                    if (*(pWeight + index) == 0.0f)
                        break;

                *(pWeight + index) = vertexWeight.mWeight;
                *(pBoneId + index) = boneId;
            }
            std::string boneName = bone->mName.C_Str();
            auto it = m_nodeLookupTable.find(boneName);
#ifdef _DEBUG
            assert(it != m_nodeLookupTable.end());
#endif
            Matrix4 offset(&bone->mOffsetMatrix.a1);
            offset = three::transpose(offset);
            mesh.bones.push_back({boneName, it->second, offset});
        }
    }

    // test
#ifdef _DEBUG
    for (const auto& weight : mesh.weights)
    {
        float sum = weight.x + weight.y + weight.z + weight.w;
        static const float error = 0.0001f;
        assert(std::abs(sum - 1.0f) < error);
    }
#endif

    mesh.indices.reserve(aimesh->mNumFaces * 3);
    for (uint32_t i = 0; i < aimesh->mNumFaces; ++i)
    {
        aiFace& face = aimesh->mFaces[i];
        assert(face.mNumIndices == 3);
        mesh.indices.push_back(face.mIndices[0]);
        mesh.indices.push_back(face.mIndices[1]);
        mesh.indices.push_back(face.mIndices[2]);
    }
    // mesh.materialIndex = aimesh->mMaterialIndex;
}

std::shared_ptr<Node> AssimpLoader::processNode(aiNode* ainode, std::shared_ptr<Node> parent)
{
    std::shared_ptr<Node> node(new Node());
    if (parent != nullptr)
        node->parent = parent;

    node->name = ainode->mName.C_Str();

    m_nodeLookupTable.insert({ node->name, node.get() });

    node->transform = transpose(Matrix4(&ainode->mTransformation.a1));

    for (uint32_t childIndex = 0; childIndex < ainode->mNumChildren; ++childIndex)
    {
        aiNode* child = ainode->mChildren[childIndex];
        node->children.push_back(processNode(child, node));
    }

    return node;
}

void AssimpLoader::processAnimation(Scene& scene, const aiAnimation* aianim)
{
    double ticksPersecond = aianim->mTicksPerSecond;
    double duration = aianim->mDuration;
    Animation& anim = scene.anim;
    anim.durationInseconds = duration / ticksPersecond;
    anim.nodeAnims.reserve(aianim->mNumChannels);
    for (uint32_t channelIndex = 0; channelIndex < aianim->mNumChannels; ++channelIndex)
    {
        aiNodeAnim* channel = aianim->mChannels[channelIndex];
        std::string nodeName = channel->mNodeName.C_Str();
        auto it = m_nodeLookupTable.find(nodeName);
#ifdef _DEBUG
        assert(it != m_nodeLookupTable.end());
#endif
        Node* node = it->second;

        NodeAnimation nodeAnim;
        nodeAnim.boneNode = node;
#ifdef _DEBUG
        assert(channel->mNumPositionKeys == channel->mNumRotationKeys);
        assert(channel->mNumRotationKeys == channel->mNumScalingKeys);
#endif
        for (unsigned int frameIndex = 0; frameIndex < channel->mNumPositionKeys; ++frameIndex)
        {
            const auto& position = channel->mPositionKeys[frameIndex];
            const auto& scale = channel->mScalingKeys[frameIndex];
            const auto& rotation = channel->mRotationKeys[frameIndex];
            const double positionTime = position.mTime;
#ifdef _DEBUG
            const double scaleTime = scale.mTime;
            const double rotationTime = rotation.mTime;
            assert(positionTime == scaleTime);
            assert(positionTime == rotationTime);
#endif
            nodeAnim.keyframes.push_back({
                Vector3(position.mValue.x, position.mValue.y, position.mValue.z),
                Vector3(scale.mValue.x, scale.mValue.y, scale.mValue.z),
                Quaternion(rotation.mValue.x, rotation.mValue.y, rotation.mValue.z, rotation.mValue.w),
                positionTime / ticksPersecond
            });
        }
        anim.nodeAnims.push_back(nodeAnim);
    }
}

#if 0

void AssimpLoader::processMaterial(aiMaterial* material, const std::string& dir)
{
    SceneMaterial* mat = new SceneMaterial();
    mat->name = material->GetName().C_Str();
    std::cout << "Loading " << mat->name << std::endl;
    aiString path;
    if (material->GetTexture(aiTextureType_AMBIENT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
    {
        std::cout << "Albedo map found: " << path.C_Str() << std::endl;
        mat->albedoPath = path.C_Str();
    }
    if (material->GetTexture(aiTextureType_DISPLACEMENT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
    {
        std::cout << "Normal map found: " << path.C_Str() << std::endl;
        mat->normalPath = path.C_Str();
    }

    m_scene->materials.push_back(std::unique_ptr<SceneMaterial>(mat));
}
#endif

} // namespace hguo