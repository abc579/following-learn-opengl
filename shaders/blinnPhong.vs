#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;

void main() {
    vs_out.fragPos = aPos;
    vs_out.normal = aNormal;
    vs_out.texCoords = aTexCoords;

    gl_Position = projection * view * vec4(aPos, 1.0);
}
