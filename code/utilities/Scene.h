#pragma once
#include "renderer/Maths.h"
#include <string>
#include <vector>
#include <memory>

namespace hguo {

struct Material
{
    std::string name;
    std::string diffuse;
};

struct Node
{
    std::string name;
    Matrix4 transform; // local transformation
    Matrix4 worldTransform; // world transformation

    std::shared_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> children;

    void calWorldTransform()
    {
        if (parent != nullptr)
        {
            worldTransform = parent->worldTransform * transform;
        }
        else
        {
            worldTransform = transform;
        }
        for (auto& child : children)
        {
            child->calWorldTransform();
        }
    }
};

struct Bone
{
    // animation
    std::string name;
    Node* boneNode;
    Matrix4 offsetMatrix;
};

struct Keyframe
{
    Vector3 position;
    Vector3 scale;
    Quaternion rotation;
    double timeInSeconds;
};

struct NodeAnimation
{
    std::vector<Keyframe> keyframes;
    Node* boneNode;
};

struct Animation
{
    double durationInseconds;
    std::vector<NodeAnimation> nodeAnims;
};

struct Mesh
{
    std::string name;
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector4> weights;
    std::vector<Vector4i> boneIds;
    std::vector<unsigned int> indices;

    std::vector<Bone> bones;
};

struct Scene
{
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::shared_ptr<Node> root;
    Animation anim;
};

} // namespace hguo
