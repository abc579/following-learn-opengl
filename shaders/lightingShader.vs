#version 330 core

out vec3 Normal;
out vec3 FragmentWorldSpaceCoordinates;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragmentWorldSpaceCoordinates = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;

    gl_Position = projection * view * vec4(FragmentWorldSpaceCoordinates, 1.0);
}
