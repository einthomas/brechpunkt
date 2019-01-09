#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 vertexPosition[];
in vec3 vertexUvw[];
flat in float vertexLayer[];

out vec3 uvw;

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Layer = int(vertexLayer[0]);
        gl_Position = vec4(vertexPosition[i], 0.0, 1.0);
        uvw = vertexUvw[i];
        EmitVertex();
    }
    EndPrimitive();
}
