#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} fs_in;

#define LIGHTS_LEN 4

struct Light {
    vec3 position;
    vec3 colour;
};

uniform sampler2D diffuseTexture;
uniform vec3 viewPosition;
uniform Light lights[LIGHTS_LEN];

void main() {
    vec3 colour = texture(diffuseTexture, fs_in.texCoords).rgb;
    vec3 normal = normalize(fs_in.normal);

    // Ambient
    vec3 ambient = .01f * colour;

    // Lighting
    vec3 lighting = vec3(0.f);
    vec3 viewDirection = normalize(viewPosition - fs_in.fragPos);
    for(int i = 0; i < LIGHTS_LEN; ++i) {
        vec3 lightDirection = normalize(lights[i].position - fs_in.fragPos);
        float diffuseFactor = max(dot(lightDirection, normal), 0.f);
        vec3 result = lights[i].colour * diffuseFactor * colour;
        // NOTE: does the order matter if we're doing length?
        float dist = length(fs_in.fragPos - lights[i].position);
        result *= 1.f / (dist * dist);
        lighting += result;
    }

    vec3 result = ambient + lighting;

    // Check if result is higher than some threshold, if so, output to BrightColor.
    float brightness = dot(result, vec3(.2126f, .7152f, .0722f));
    if(brightness > 1.f) {
        BrightColor = vec4(result, 1.f);
    } else {
        BrightColor = vec4(vec3(0.f), 1.f);
    }

    FragColor = vec4(result, 1.f);
}
