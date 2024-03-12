#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 viewerPosition;
uniform samplerCube skybox;

void main() {
    const float ratio = 1.00 / 2.42;
    vec3 I = normalize(Position - viewerPosition);
    // vec3 R = reflect(I, normalize(Normal));
    vec3 R = refract(I, normalize(Normal), ratio);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}

