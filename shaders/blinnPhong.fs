#version 330 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} fs_in;

out vec4 FragColor;

uniform sampler2D planeTexture;
uniform vec3 lightPosition;
uniform vec3 viewerPosition;
uniform bool blinn;

void main() {
    vec3 colour = texture(planeTexture, fs_in.texCoords).rgb;

    vec3 ambient = colour * .1f;

    vec3 lightDirection = normalize(lightPosition - fs_in.fragPos);
    vec3 normal = normalize(fs_in.normal);

    float diffuseFactor = max(dot(lightDirection, normal), 0.0f);
    vec3 diffuse = diffuseFactor * colour;

    vec3 viewerDirection = normalize(viewerPosition - fs_in.fragPos);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float specularFactor = 0.f;
    if(blinn) {
        vec3 halfwayDirection = normalize(lightDirection + viewerDirection);
        specularFactor = pow(max(dot(normal, halfwayDirection), 0.f), 32.f);
    } else {
        specularFactor = pow(max(dot(viewerDirection, reflectDirection), 0.f), 8.f);
    }
    vec3 specular = vec3(.3f) * specularFactor; // Assuming white bright colour.

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
