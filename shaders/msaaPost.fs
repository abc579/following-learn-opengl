#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    FragColor = texture(screenTexture, TexCoords);
    float average = .2126f * FragColor.r + .7152f * FragColor.g + .0722 * FragColor.b;
    FragColor = vec4(average, average, average, 1.0);
}

