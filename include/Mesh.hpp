#pragma once

#include <Shader.hpp>

enum class TextureType {
    DIFFUSE,
    SPECULAR
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoordinates;
};

struct Texture {
    unsigned int id;
    TextureType textureType;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<Texture> textures;
    std::vector<unsigned int> indices;

    explicit Mesh(const std::vector<Vertex>& aVertices, const std::vector<Texture>& aTextures,
                  const std::vector<unsigned int>& aIndices);

    void draw(Shader& shader);

private:
    unsigned int VAO{ 0 };
    unsigned int VBO{ 0 };
    unsigned int EBO{ 0 };

    void setupMesh();
};
