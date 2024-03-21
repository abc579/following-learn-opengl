#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPositionWorldSpace;
    vec2 TexCoords;
    vec3 TangentLightPosition;
    vec3 TangentViewPosition;
    vec3 TangentFragPosition;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPosition;
uniform vec3 viewerPosition;

void main() {
    vs_out.FragPositionWorldSpace = vec3(model * vec4(aPos, 1.f));
    vs_out.TexCoords = aTexCoords;

    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    vec3 N = normalize(mat3(model) * aNormal);

    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPosition = TBN * lightPosition;
    vs_out.TangentViewPosition = TBN * viewerPosition;
    vs_out.TangentFragPosition = TBN * vs_out.FragPositionWorldSpace;

    gl_Position = projection * view * model * vec4(aPos, 1.f);
}
