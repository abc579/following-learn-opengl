#version 330 core

in vec2 TexCoords;
in vec3 FragmentWorldSpaceCoordinates;
in vec3 Normal;

out vec4 FragColor;

struct DirectionalLight {
    vec3 position;
    vec3 colour;
    vec3 ambient;
    vec3 specular;
    vec3 diffuse;
};

struct SpotlightLight {
    vec3 position;
    vec3 direction;
    vec3 colour;
    vec3 ambient;
    vec3 specular;
    vec3 diffuse;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_shininess0;

uniform DirectionalLight directionalLight;
uniform SpotlightLight spotlightLight;

uniform vec3 viewerPosition;

vec3 gNormal = normalize(Normal);

vec3 computeDirectionalLight(DirectionalLight light);
vec3 computeSpotlightLight(SpotlightLight light);

void main() {
    // vec3 result = computeDirectionalLight(directionalLight);

    // result += computeSpotlightLight(spotlightLight);

    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

vec3 computeDirectionalLight(DirectionalLight light) {
    vec3 ambient = (light.ambient * light.colour) * vec3(texture(texture_diffuse0, TexCoords));

    vec3 lightDirection = normalize(light.position - FragmentWorldSpaceCoordinates);
    float diffuseFactor = max(dot(gNormal, lightDirection), 0.0);

    vec3 diffuse = (light.diffuse * light.colour) * diffuseFactor * vec3(texture(texture_diffuse0, TexCoords));

    vec3 reflectionDirection = reflect(-lightDirection, gNormal);
    vec3 viewerDirection = normalize(viewerPosition - FragmentWorldSpaceCoordinates);
    float specularFactor = pow(max(dot(viewerDirection, reflectionDirection), 0.0), 32);
    vec3 specular = (light.specular * light.colour) * specularFactor * vec3(texture(texture_specular0, TexCoords));

    return ambient + diffuse + specular;
}

vec3 computeSpotlightLight(SpotlightLight light) {
    float dist = length(light.position - FragmentWorldSpaceCoordinates);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    vec3 ambient = (light.ambient * light.colour) * vec3(texture(texture_diffuse0, TexCoords));
    ambient *= attenuation;

    vec3 lightDirection = normalize(light.position - FragmentWorldSpaceCoordinates);
    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(gNormal, lightDirection), 0.0);
    vec3 diffuse = (light.diffuse * light.colour) * diff * vec3(texture(texture_diffuse0, TexCoords));
    diffuse *= attenuation;
    diffuse *= intensity;

    vec3 viewerDirection = normalize(viewerPosition - FragmentWorldSpaceCoordinates);
    vec3 reflectionDirection = reflect(-lightDirection, gNormal);
    float spec = pow(max(dot(viewerDirection, reflectionDirection), 0.0), 32);
    vec3 specular = (light.specular * light.colour) * spec * vec3(texture(texture_specular0, TexCoords));
    specular *= attenuation;
    specular *= intensity;

    return ambient + diffuse + specular;
}
