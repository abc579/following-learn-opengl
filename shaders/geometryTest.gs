#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float magnitude = .4;

uniform mat4 projection;

void generateLine(int index) {
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();

    gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * magnitude);
    EmitVertex();

    EndPrimitive();
}

void main() {
    generateLine(0); // first vertex normal
    generateLine(1); // second
    generateLine(2); // third
}
