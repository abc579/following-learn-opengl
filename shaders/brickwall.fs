#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPositionWorldSpace;
    vec2 TexCoords;
    vec3 TangentLightPosition;
    vec3 TangentViewPosition;
    vec3 TangentFragPosition;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

uniform vec3 lightPosition;
uniform vec3 viewerPosition;
uniform float heightScale;

vec2 parallaxMapping(vec3 viewerDirection);

void main() {
    vec3 viewerDirection = normalize(fs_in.TangentViewPosition - fs_in.TangentFragPosition);

    vec2 texCoords = parallaxMapping(viewerDirection);
    if(texCoords.x > 1.f || texCoords.y > 1.f || texCoords.x < 0.f || texCoords.y < 0.f) {
        discard;
    }

    // Get normal from normal map, range [0, 1]
    vec3 normal = texture(normalTexture, texCoords).rgb;

    // Transform normal vector to range [-1, 1]
    normal = normalize(normal * 2.f - 1.f);

    vec3 colour = texture(diffuseTexture, texCoords).rgb;

    vec3 ambient = .1f * colour;

    vec3 lightDirection = normalize(fs_in.TangentLightPosition - fs_in.TangentFragPosition);
    float diffuseFactor = max(dot(lightDirection, normal), 0.f);
    vec3 diffuse = diffuseFactor * colour;

    vec3 reflectDirection = reflect(-lightDirection, normal);
    vec3 halfwayDirection = normalize(lightPosition + viewerPosition);

    float specularFactor = pow(max(dot(normal, halfwayDirection), 0.f), 32.f);
    vec3 specular = vec3(0.2f) * specularFactor;

    FragColor = vec4(ambient + diffuse + specular, 1.f);
}

// Returns the displaced texture coordinates
vec2 parallaxMapping(vec3 viewerDirection) {
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(minLayers, maxLayers, max(dot(vec3(0.f, 0.f, 1.f), viewerDirection), 0.f));

    float sizeLayer = 1.f / numLayers;
    float currentLayerDepth = 0.f;

    vec2 P = viewerDirection.xy * heightScale;
    vec2 deltaTexCoordsOffset = P / numLayers;

    vec2 currentTexCoords = fs_in.TexCoords;
    float currentDepthValue = texture(depthTexture, currentTexCoords).r;

    while(currentDepthValue > currentLayerDepth) {
        // Shift texture coordinates along direction of P.
        currentTexCoords = currentTexCoords - deltaTexCoordsOffset;
        // Get current depth map value.
        currentDepthValue = texture(depthTexture, currentTexCoords).r;
        // Get current layer depth.
        currentLayerDepth += sizeLayer;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoordsOffset;

    // Get depth after and before collision for linear interpolation.
    float prevDepthValue = texture(depthTexture, prevTexCoords).r - currentLayerDepth + sizeLayer;
    float afterDepthValue = currentDepthValue - currentLayerDepth;

    // Interpolation of texture coordinates.
    float weight = afterDepthValue / (afterDepthValue - prevDepthValue);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.f - weight);

    return finalTexCoords;
}
