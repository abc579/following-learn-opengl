#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} fs_in;

uniform vec3 lightColour;

void main() {
    FragColor = vec4(lightColour, 1.f);
    float brightness = dot(FragColor.rgb, vec3(.2126f, .7152f, .0722f));
    if(brightness > 1.f) {
        BrightColor = vec4(FragColor.rgb, 1.f);
    } else {
        BrightColor = vec4(vec3(0.f), 1.f);
    }
}
