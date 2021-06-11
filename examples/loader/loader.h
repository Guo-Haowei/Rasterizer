#pragma once
#include <string>
#include <unordered_map>
#include "Scene.h"
#include "rasterizer/Texture.h"

struct aiMesh;
struct aiNode;
struct aiMaterial;
struct aiScene;
struct aiAnimation;

namespace rs {

extern void loadTexture(Texture& texture, const char* path);

class AssimpLoader {
   public:
    Scene load(const char* path);

   private:
    std::shared_ptr<Node> processNode(aiNode* ainode, std::shared_ptr<Node> parent);

    void processMesh(Scene& scene, const aiMesh* aimesh);

    void processAnimation(Scene& scene, const aiAnimation* aianim);

    void processMaterial(aiMaterial* material, const std::string& dir);

    std::unordered_map<std::string, Node*> m_nodeLookupTable;
};

}  // namespace rs
