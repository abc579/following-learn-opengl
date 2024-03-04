#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragmentWorldSpaceCoordinates;
out vec2 TexCoords;

void main() {
    FragmentWorldSpaceCoordinates = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;

    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragmentWorldSpaceCoordinates, 1.0);
}
