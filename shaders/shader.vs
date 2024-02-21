#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform vec1 horizontalOffsetUniform;

out vec3 ourColor;

void main() {
    gl_Position = vec4(-position.x - horizontalOffsetUniform, -position.y, position.z, 1.0);
    ourColor = color;
}
