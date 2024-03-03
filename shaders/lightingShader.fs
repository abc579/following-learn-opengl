#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 specular;
    vec3 diffuse;
};

in vec3 Normal;
in vec3 FragmentWorldSpaceCoordinates;

out vec4 FragColor;

uniform vec3 objectColour;
uniform vec3 lightColour;
uniform vec3 lightPosition;
uniform vec3 viewerPosition;

uniform Material material;
uniform Light light;

void main() {
    // Ambient
    vec3 ambient = light.ambient * material.ambient;

    // Diffuse
    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightPosition - FragmentWorldSpaceCoordinates);
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // Specular
    vec3 viewerDirection = normalize(viewerPosition - FragmentWorldSpaceCoordinates);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewerDirection, reflectionDirection), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
