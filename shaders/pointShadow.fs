#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main() {
    // TODO: try to change the order and see if it breaks.
    float lightDistance = length(FragPos.xyz - lightPos);
    // float lightDistance = length(lightPosition - FragPos.xyz);

    // map to [0, 1]
    lightDistance = lightDistance / far_plane;

    gl_FragDepth = lightDistance;
}
