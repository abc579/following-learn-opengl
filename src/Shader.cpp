#include <Shader.hpp>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    // Read file contents.
    std::ifstream vertexStreamFile(vertexPath);
    if(!vertexStreamFile.is_open()) {
        std::cerr << "Could not open vertex source code. Path: " << vertexPath << '\n';
    }

    std::ifstream fragmentStreamFile(fragmentPath);
    if(!fragmentStreamFile.is_open()) {
        std::cerr << "Could not open fragment source code. Path: " << fragmentPath << '\n';
    }

    std::stringstream vertexSStream;
    std::string vertexSourceCode;
    vertexSStream << vertexStreamFile.rdbuf();
    vertexSourceCode = vertexSStream.str();

    std::stringstream fragmentSStream;
    std::string fragmentSourceCode;
    fragmentSStream << fragmentStreamFile.rdbuf();
    fragmentSourceCode = fragmentSStream.str();

    const char* vertexSourceCodeC = vertexSourceCode.c_str();
    const char* fragmentSourceCodeC = fragmentSourceCode.c_str();

    // Compile and link shaders.
    int success;
    std::array<char, 512> log;

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSourceCodeC, nullptr);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertex, log.size(), nullptr, &log[0]);
        std::cerr << "Could not compile vertex shader: " << &log[0] << '\n';
    }

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSourceCodeC, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragment, log.size(), nullptr, &log[0]);
        std::cerr << "Could not compile fragment shader: " << &log[0] << '\n';
    }

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(id, log.size(), nullptr, &log[0]);
        std::cerr << "Could not link vertex and fragment shaders: " << &log[0] << '\n';
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() {
    glUseProgram(id);
}

void Shader::setUniformBool(const std::string& name, bool val) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)val);
}

void Shader::setUniformInt(const std::string& name, int val) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), val);
}

void Shader::setUniformFloat(const std::string& name, float val) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), val);
}

void Shader::setMat4(const std::string& name, const glm::mat4& val) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &val[0][0]);
}

void Shader::setVec3(const std::string& name, float valX, float valY, float valZ) const {
    glUniform3f(glGetUniformLocation(id, name.c_str()), valX, valY, valZ);
}
