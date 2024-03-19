#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

uniform vec3 lightPosition;
uniform vec3 viewerPosition;

uniform float farPlane;

float computeShadow(vec3 fragPos) {
    vec3 fragmentToLight = fragPos - lightPosition;
    float closestDepth = texture(depthMap, fragmentToLight).r;
    closestDepth *= farPlane;
    float currentDepth = length(fragmentToLight);
    float bias = .05f;
    float shadow = currentDepth - bias > closestDepth ? 1.f : 0.f;

    return shadow;
}

void main() {
    vec3 colour = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColour = vec3(.3f);

    // Ambient
    vec3 ambient = .3f * colour;

    // Diffuse
    vec3 lightDirection = normalize(lightPosition - fs_in.FragPos);
    float diffuseFactor = max(dot(lightDirection, normal), 0.f);
    vec3 diffuse = diffuseFactor * lightColour;

    // Specular
    vec3 viewerDirection = normalize(viewerPosition - fs_in.FragPos);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    vec3 halfwayDirection = normalize(lightDirection + viewerDirection);
    float specularFactor = pow(max(dot(normal, halfwayDirection), 0.f), 64.f);
    vec3 specular = specularFactor * lightColour;

    // Shadow
    float shadowFactor = computeShadow(fs_in.FragPos);
    vec3 lighting = (ambient + (1.f - shadowFactor) * (diffuse + specular)) * colour;

    FragColor = vec4(lighting, 1.f);
}
