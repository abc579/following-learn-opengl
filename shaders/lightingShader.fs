#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 colour;
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 specular;
    vec3 diffuse;

    float constant;
    float linear;
    float quadratic;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragmentWorldSpaceCoordinates;

uniform vec3 viewerPosition;
uniform Material material;
uniform Light light;

out vec4 FragColor;

void main() {
    // Distance from fragment to lightPosition
    float dist = length(light.position - FragmentWorldSpaceCoordinates);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    // Ambient (assuming it's white)
    vec3 ambient = (light.ambient * light.colour) * vec3(texture(material.diffuse, TexCoords));
    ambient *= attenuation;

    // Light direction.
    vec3 lightDirection = normalize(light.position - FragmentWorldSpaceCoordinates);
    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Diffuse
    vec3 normal = normalize(Normal);
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = (light.diffuse * light.colour) * diff * vec3(texture(material.diffuse, TexCoords));
    diffuse *= attenuation;
    diffuse *= intensity;

    // Specular
    vec3 viewerDirection = normalize(viewerPosition - FragmentWorldSpaceCoordinates);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewerDirection, reflectionDirection), 0.0), material.shininess);
    vec3 specular = (light.specular * light.colour) * spec * vec3(texture(material.specular, TexCoords));
    specular *= attenuation;
    specular *= intensity;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
