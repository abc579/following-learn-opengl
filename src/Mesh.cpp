#include <Mesh.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

Mesh::Mesh(const std::vector<Vertex>& aVertices, const std::vector<Texture>& aTextures,
           const std::vector<unsigned int>& aIndices)
    :vertices(aVertices), textures(aTextures), indices(aIndices)
{
    setupMesh();
}

void Mesh::draw(Shader& shader) {
    unsigned int diffuseNumber{ 0 };
    unsigned int specularNumber{ 0 };

    for(unsigned int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string texTypeStr;

        if(textures[i].textureType == TextureType::DIFFUSE) {
            texTypeStr = "texture_diffuse" + diffuseNumber;
            ++diffuseNumber;
        } else if(textures[i].textureType == TextureType::SPECULAR) {
            texTypeStr = "texture_specular" + specularNumber;
            ++specularNumber;
        }

        shader.setUniformFloat("material." + texTypeStr, static_cast<float>(i));
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // Do I need to call glActiveTexture(GL_TEXTURE0); ?

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<int>(vertices.size()) * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<int>(indices.size()) * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex positions.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex normals.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Vertex texture coords.
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoordinates));
    glBindVertexArray(0);
}
