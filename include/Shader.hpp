#pragma once

#include <string>

#include <glm/mat4x4.hpp>

struct Shader {
    unsigned int id;

    explicit Shader(const std::string& vertexPath, const std::string& fragmentPath);
    explicit Shader(const std::string& vertexPath, const std::string& geometryPath, const std::string& fragmentPath);

    void use();
    void setUniformBool(const std::string& name, bool val) const;
    void setUniformInt(const std::string& name, int val) const;
    void setUniformFloat(const std::string& name, float val) const;
    void setMat4(const std::string& name, const glm::mat4& val) const;
    void setVec3(const std::string& name, float valX, float valY, float valZ) const;
    void setVec3(const std::string& name, const glm::vec3& val) const;
    void setVec2(const std::string& name, const glm::vec2& val) const;
};
