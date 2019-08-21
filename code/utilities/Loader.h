#pragma once
#include "Scene.h"
#include "renderer/Texture.h"
#include <unordered_map>
#include <string>

struct aiMesh;
struct aiNode;
struct aiMaterial;
struct aiScene;
struct aiAnimation;

namespace hguo {

extern void loadTexture(Texture& texture, const char* path);

class AssimpLoader
{
public:
    Scene load(const char* path);

private:
    std::shared_ptr<Node> processNode(aiNode* ainode, std::shared_ptr<Node> parent);

    void processMesh(Scene& scene, const aiMesh* aimesh);

    void processAnimation(Scene& scene, const aiAnimation* aianim);

    void processMaterial(aiMaterial* material, const std::string& dir);

    std::unordered_map<std::string, Node*> m_nodeLookupTable;
};

} // namespace hguo
