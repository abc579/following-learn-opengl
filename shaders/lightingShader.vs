#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPosition;
uniform vec3 viewerPosition;
uniform vec3 lightColour;

out vec3 result;

void main() {
    vec3 FragmentViewSpaceCoordinates = vec3(view * model * vec4(position, 1.0));
    vec3 LightPositionViewSpaceCoordinates = vec3(view * model * vec4(lightPosition, 1.0));
    vec3 ViewerPositionViewSpaceCoordinates = vec3(view * model * vec4(viewerPosition, 1.0));
    vec3 Normal = normalize(mat3(transpose(inverse(view * model))) * normal);

    // Ambient
    const float ambientFactor = .1f;
    vec3 ambient = ambientFactor * lightColour;

    // Diffuse
    vec3 lightDirection = normalize(LightPositionViewSpaceCoordinates - FragmentViewSpaceCoordinates);
    float diff = max(dot(Normal, lightDirection), 0.0);
    vec3 diffuse = diff * lightColour;

    // Specular
    const float specularFactor = .1f;
    vec3 viewerDirection = normalize(ViewerPositionViewSpaceCoordinates - FragmentViewSpaceCoordinates);
    vec3 reflectionDirection = reflect(-lightDirection, Normal);
    float spec = pow(max(dot(viewerDirection, reflectionDirection), 0.0), 32);
    vec3 specular = specularFactor * spec * lightColour;

    result = (ambient + diffuse + specular);
    gl_Position = projection * vec4(FragmentViewSpaceCoordinates, 1.0);
}
