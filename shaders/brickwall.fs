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

uniform vec3 lightPosition;
uniform vec3 viewerPosition;

void main() {
    // Get normal from normal map, range [0, 1]
    vec3 normal = texture(normalTexture, fs_in.TexCoords).rgb;

    // Transform normal vector to range [-1, 1]
    normal = normalize(normal * 2.f - 1.f);

    vec3 colour = texture(diffuseTexture, fs_in.TexCoords).rgb;

    vec3 ambient = .1f * colour;

    vec3 lightDirection = normalize(fs_in.TangentLightPosition - fs_in.TangentFragPosition);
    float diffuseFactor = max(dot(lightDirection, normal), 0.f);
    vec3 diffuse = diffuseFactor * colour;

    vec3 viewerDirection = normalize(fs_in.TangentViewPosition - fs_in.TangentFragPosition);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    vec3 halfwayDirection = normalize(lightPosition + viewerPosition);

    float specularFactor = pow(max(dot(normal, halfwayDirection), 0.f), 32.f);
    vec3 specular = vec3(0.2f) * specularFactor;

    FragColor = vec4(ambient + diffuse + specular, 1.f);
}
