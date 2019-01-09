#version 330 core

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inUvw;
layout(location = 2) in float inLayer;

out vec2 vertexPosition;
out vec3 vertexUvw;
flat out float vertexLayer;

void main() {
    vertexPosition = inPosition;
    vertexUvw = inUvw;
    vertexLayer = inLayer;
}
