#version 330 core

out vec4 FragColor;

in vec3 result;
/*
in vec3 Normal;
in vec3 FragmentViewSpaceCoordinates;
in vec3 LightPositionViewSpaceCoordinates;
in vec3 ViewerPositionViewSpaceCoordinates;

uniform vec3 objectColour;
uniform vec3 lightColour;
*/

uniform vec3 objectColour;

void main() {
  /*
    // Ambient
    const float ambientFactor = .1f;
    vec3 ambient = ambientFactor * lightColour;

    // Diffuse
    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(LightPositionViewSpaceCoordinates - FragmentViewSpaceCoordinates);
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * lightColour;

    // Specular
    const float specularFactor = .5f;
    vec3 viewerDirection = normalize(ViewerPositionViewSpaceCoordinates - FragmentViewSpaceCoordinates);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewerDirection, reflectionDirection), 0.0), 32);
    vec3 specular = specularFactor * spec * lightColour;
    vec3 result = (ambient + diffuse + specular) * objectColour;
*/

    FragColor = vec4(result * objectColour, 1.0);
}
