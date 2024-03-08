#include <Model.hpp>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <iostream>

#include <assimp/types.h>

static unsigned int textureFromFile(const char* path, const std::string& directory, [[maybe_unused]]bool gamma = false) {
    std::string fileName = std::string(path);
    fileName = directory + '/' + fileName;

    unsigned int textureID = 0;
    glGenTextures(1, &textureID);

    int width = 0, height = 0, numberComponents = 0;
    unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &numberComponents, 0);

    if(!data) {
        std::cerr << "::stbi_load:: ERROR LOADING TEXTURE, path -> " << path << '\n';
        stbi_image_free(data);
    }

    GLenum format = GL_RGB;
    if(numberComponents == 1) {
        format = GL_RED;
    } else if(numberComponents == 3) {
        format = GL_RGB;
    } else if(numberComponents == 4) {
        format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::draw(Shader& shader) const {
    for(const auto& mesh : meshes) {
        mesh.draw(shader);
    }
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR reading file in ASSIMP! " << importer.GetErrorString() << '\n';
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* const scene) {
    for(unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for(unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* const scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;

        vertex.position = glm::vec3(mesh->mVertices[i].x,
                                    mesh->mVertices[i].y,
                                    mesh->mVertices[i].z);

        if(mesh->HasNormals()) {
            vertex.normal = glm::vec3(mesh->mNormals[i].x,
                                      mesh->mNormals[i].y,
                                      mesh->mNormals[i].z);
        }

        // There are 8 possible texture coordinates, but we only care about the first two.
        if(mesh->mTextureCoords[0]) {
            vertex.textureCoordinates = glm::vec2(mesh->mTextureCoords[0][i].x,
                                                  mesh->mTextureCoords[0][i].y);
        } else {
            vertex.textureCoordinates = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    return Mesh(vertices, textures, indices);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type) {
    std::vector<Texture> textures;

    for(unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;

        for(unsigned int j = 0; j < texturesLoaded.size(); ++j) {
            if (std::strcmp(texturesLoaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(texturesLoaded[j]);
                skip = true;
                break;
            }
        }

        if(!skip) {
            TextureType myType = TextureType::DIFFUSE;
            switch(type) {
            case aiTextureType_DIFFUSE:
                myType = TextureType::DIFFUSE;
                break;
            case aiTextureType_SPECULAR:
                myType = TextureType::SPECULAR;
                break;
            default:
                break;
            }

            Texture t {
                .id = textureFromFile(str.C_Str(), directory),
                .textureType = myType,
                .path = std::string(str.C_Str())
            };

            textures.push_back(t);
            texturesLoaded.push_back(t);
        }
    }

    return textures;
}
