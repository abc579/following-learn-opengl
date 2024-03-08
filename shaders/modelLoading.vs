#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 FragmentWorldSpaceCoordinates;
out vec3 Normal;

void main() {
    FragmentWorldSpaceCoordinates = vec3(model * vec4(position, 1.0));
    TexCoords = aTexCoords;
    Normal = mat3(transpose(inverse(model))) * normal;

    gl_Position = projection * view * vec4(position, 1.0);
}
