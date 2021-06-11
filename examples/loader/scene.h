#pragma once
#include <memory>
#include <string>
#include <vector>
#include "common/linalg.h"

namespace rs {

struct Material {
    std::string name;
    std::string diffuse;
};

struct Node {
    std::string name;
    gfx::mat4 transform;       // local transformation
    gfx::mat4 worldTransform;  // world transformation

    std::shared_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> children;

    void calWorldTransform() {
        if (parent != nullptr) {
            worldTransform = parent->worldTransform * transform;
        } else {
            worldTransform = transform;
        }
        for (auto& child : children) {
            child->calWorldTransform();
        }
    }
};

struct Bone {
    // animation
    std::string name;
    Node* boneNode;
    gfx::mat4 offsetMatrix;
};

struct Keyframe {
    gfx::vec3 position;
    gfx::vec3 scale;
    gfx::quat rotation;
    double timeInSeconds;
};

struct NodeAnimation {
    std::vector<Keyframe> keyframes;
    Node* boneNode;
};

struct Animation {
    double durationInseconds;
    std::vector<NodeAnimation> nodeAnims;
};

struct Mesh {
    std::string name;
    std::vector<gfx::vec3> positions;
    std::vector<gfx::vec3> normals;
    std::vector<gfx::vec2> uvs;
    std::vector<gfx::vec4> weights;
    std::vector<gfx::ivec4> boneIds;
    std::vector<unsigned int> indices;

    std::vector<Bone> bones;
};

struct Scene {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::shared_ptr<Node> root;
    Animation anim;
};

}  // namespace rs
