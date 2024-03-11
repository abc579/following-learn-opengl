#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main() {
    vec4 textureColour = texture(texture1, TexCoords);

    if(textureColour.a <= 0.1) {
        discard;
    }

    FragColor = textureColour;
}

