#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} fs_in;

struct Light {
    vec3 position;
    vec3 colour;
};

uniform Light lights[16];
uniform sampler2D diffuseTexture;
uniform vec3 viewPosition;

void main() {
    vec3 colour = texture(diffuseTexture, fs_in.texCoords).rgb;
    vec3 normal = normalize(fs_in.normal);

    vec3 ambient = .1f * colour;
    vec3 lighting = vec3(0.f);

    for(int i = 0; i < 16; ++i) {
        vec3 lightDirection = normalize(lights[i].position - fs_in.fragPos);
        float diffuseFactor = max(dot(lightDirection, normal), 0.f);
        vec3 diffuse = lights[i].colour * diffuseFactor;
        vec3 result = diffuse;
        // attenuation, use quadratic as we have gamma correction
        float distance = length(fs_in.fragPos - lights[i].position);
        result *= 1.f / (distance * distance);
        lighting += result;
    }

    FragColor = vec4(ambient + lighting, 1.f);
}
