#version 330 core

in vec4 FragPos;

uniform vec3 lightPosition;
uniform float farPlane;

void main() {
    // TODO: try to change the order and see if it breaks.
    float lightDistance = length(FragPos.xyz - lightPosition);
    // float lightDistance = length(lightPosition - FragPos.xyz);

    // map to [0, 1]
    lightDistance = lightDistance / farPlane;

    gl_FragDepth = lightDistance;
}
