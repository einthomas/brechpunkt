#version 330 core

layout(location = 0) out vec4 positionOut;
layout(location = 1) out vec4 normalOut;

in vec3 worldPos;
in vec3 normal;

void main() {
    normalOut = vec4(normal, 1.0f);
    positionOut = vec4(worldPos, 1.0f);
}
