#version 330 core

layout(location = 0) in vec3 _pos;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec2 _texCoord;

uniform mat4 model;
uniform mat4 viewProjectionMatrix;

out vec3 worldPos;
out vec3 normal;
out vec2 texCoord;

void main() {
    vec4 worldPos4 = model * vec4(_pos, 1.0f);
    worldPos = worldPos4.xyz;
    normal = _normal;
    texCoord = _texCoord;
    gl_Position = viewProjectionMatrix * worldPos4;
}
