#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
    vec4 fragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPosition;
uniform vec3 viewerPosition;

float shadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDirection) {
    // Work with range [-1, 1].
    vec3 projectionCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform NDC to range [0, 1].
    projectionCoords = projectionCoords * 0.5f + 0.5f;

    if(projectionCoords.z > 1.f) {
        return 0.f;
    }

    float closestDepth = texture(shadowMap, projectionCoords.xy).r;
    float currentDepth = projectionCoords.z;

    float shadow = 0.f;
    float bias = max(.05f * (1.f - dot(normal, lightDirection)), .005f);
    vec2 texelSize = 1.f / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projectionCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.f : 0.f;
        }
    }

    return shadow / 9.f; // 9 -> number of iterations
}

void main() {
    vec3 colour = texture(diffuseTexture, fs_in.texCoords).rgb;
    vec3 normal = normalize(fs_in.normal);
    vec3 lightColour = vec3(.3f);

    // Ambient
    vec3 ambient = .3f * lightColour;

    // Diffuse
    vec3 lightDirection = normalize(lightPosition - fs_in.fragPos);
    float diffuseFactor = max(dot(lightDirection, normal), 0.f);
    vec3 diffuse = diffuseFactor * lightColour;

    // Specular
    vec3 viewerDirection = normalize(viewerPosition - fs_in.fragPos);
    vec3 halfwayDirection = normalize(lightDirection + viewerDirection);
    float specularFactor = pow(max(dot(normal, halfwayDirection), 0.f), 64.f);
    vec3 specular = specularFactor * lightColour;

    // Shadow
    float shadowFactor = shadowCalculation(fs_in.fragPosLightSpace, normal, lightDirection);
    vec3 lighting = (ambient + (1.f - shadowFactor) * (diffuse + specular)) * colour;

    FragColor = vec4(lighting, 1.f);
}
