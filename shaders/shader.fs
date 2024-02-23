#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoordsOut;

uniform sampler2D ourTexture;
uniform sampler2D ourTexture2;
uniform float blendFactor;

void main() {
    FragColor = mix(texture(ourTexture, texCoordsOut), texture(ourTexture2, texCoordsOut), blendFactor);
}
