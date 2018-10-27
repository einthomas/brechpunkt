#version 330 core

layout(location = 0) in vec3 _pos;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec2 _texCoord;

uniform mat4 model;
uniform mat4 view;

out vec3 vertexPosition;
out vec2 vertexTexCoord;
out vec3 vertexNormal;

void main() {
    vertexTexCoord = _texCoord;
    vertexNormal = _normal;
    vertexPosition = (view * model * vec4(_pos, 1.0f)).xyz;
}
