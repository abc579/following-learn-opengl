#pragma once

#include <string>

struct Shader {
    unsigned int id;

    explicit Shader(const std::string& vertexPath, const std::string& fragmentPath);

    void use();
    void setUniformBool(const std::string& name, bool val) const;
    void setUniformInt(const std::string& name, int val) const;
    void setUniformFloat(const std::string& name, float val) const;
};
