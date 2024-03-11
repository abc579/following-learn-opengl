#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main() {
    vec3 colour = texture(texture1, TexCoords).rgb;
    FragColor = vec4(colour, 1.0);
}

