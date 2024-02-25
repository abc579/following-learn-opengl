#pragma once

#include <string>
#include <glm/glm.hpp>

struct Shader {
    unsigned int id;

    explicit Shader(const std::string& vertexPath, const std::string& fragmentPath);

    void use();
    void setUniformBool(const std::string& name, bool val) const;
    void setUniformInt(const std::string& name, int val) const;
    void setUniformFloat(const std::string& name, float val) const;
    void setMat4(const std::string& name, const glm::mat4& val) const;
};
