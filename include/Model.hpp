#pragma once

#include <Mesh.hpp>
#include <Shader.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

class Model {
public:
    explicit Model(const std::string& path);

    void draw(Shader& shader) const;

    std::vector<Mesh> meshes;
    std::vector<Texture> texturesLoaded;
private:
    std::string directory;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* const scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* const scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type);
};
