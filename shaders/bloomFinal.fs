#version 330 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;

void main() {
    const float gamma = 2.2f;

    vec3 hdrColour = texture(scene, texCoords).rgb;
    vec3 bloomColour = texture(bloomBlur, texCoords).rgb;
    if(bloom) {
        hdrColour += bloomColour;
    }

    // Tone mapping
    vec3 result = vec3(1.f) - exp(-hdrColour * exposure);
    // And gamma correct.
    result = pow(result, vec3(1.f / gamma));

    FragColor = vec4(result, 1.f);
}
