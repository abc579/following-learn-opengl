#version 330 core

out vec4 FragColor;

in vec3 fColour;

void main() {
    FragColor = vec4(fColour, 1.0);
}
