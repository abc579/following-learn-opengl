#version 330 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;

void main() {
    const float gamma = 2.2f;
    vec3 hdrColour = texture(hdrBuffer, texCoords).rgb;
    if(hdr) {
        vec3 result = vec3(1.f) - exp(-hdrColour * exposure);
        // gamma correct this
        result = pow(result, vec3(1.f / gamma));
        FragColor = vec4(result, 1.f);
    } else {
        vec3 result = pow(hdrColour, vec3(1.f / gamma));
        FragColor = vec4(result, 1.f);
    }
}
