#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 model;

// Transforms every vertex to world-space coordinates. We pass that information
// to the geometry shader.
void main() {
    gl_Position = model * vec4(aPos, 1.f);
}
