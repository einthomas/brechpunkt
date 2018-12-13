#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

in vec3 vertexPosition[];
in vec2 vertexTexCoord[];
in vec3 vertexNormal[];

out vec2 geometryTexCoord;
out vec3 geometryNormal;

/*  https://www.scratchapixel.com/lessons/3d-basic-rendering/
    perspective-and-orthographic-projection-matrix/
    opengl-perspective-projection-matrix
*/
float near = 0.1f, far = 100.0f;
mat4 projection = mat4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, -(far + near) / (far - near), -1,
    0, 0, -2 * far * near / (far - near), 0
);

void face(ivec3 swizzle, vec3 signs) {
    for (int j = 0; j < 3; j++) {
        vec3 p = vertexPosition[j];
        geometryTexCoord = vertexTexCoord[j];
        geometryNormal = vertexNormal[j];
        gl_Position = projection * vec4(
            vec3(p[swizzle.x], p[swizzle.y], p[swizzle.z]) * signs, 1.0
        );
        EmitVertex();
    }
    EndPrimitive();
}

void main() {

    gl_Layer = 0;
    face(ivec3(2, 1, 0), vec3(-1, -1, -1));

    gl_Layer = 1;
    face(ivec3(2, 1, 0), vec3(1, -1, 1));

    gl_Layer = 2;
    face(ivec3(0, 2, 1), vec3(1, 1, -1));

    gl_Layer = 3;
    face(ivec3(0, 2, 1), vec3(1, -1, 1));

    gl_Layer = 4;
    face(ivec3(0, 1, 2), vec3(1, -1, -1));

    gl_Layer = 5;
    face(ivec3(0, 1, 2), vec3(-1, -1, 1));
}
