#version 330 core

out vec3 Normal;
out vec3 FragmentViewSpaceCoordinates;
out vec3 LightPositionViewSpaceCoordinates;
out vec3 ViewerPositionViewSpaceCoordinates;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform vec3 lightPosition;
uniform vec3 viewerPosition;
uniform mat4 projection;

void main() {
    FragmentViewSpaceCoordinates = vec3(view * model * vec4(position, 1.0));
    LightPositionViewSpaceCoordinates = vec3(view * model * vec4(lightPosition, 1.0));
    ViewerPositionViewSpaceCoordinates = vec3(view * model * vec4(viewerPosition, 1.0));

    Normal = mat3(transpose(inverse(view * model))) * normal;

    gl_Position = projection * vec4(FragmentViewSpaceCoordinates, 1.0);
}
