#version 330 core

layout(location = 0) in vec3 _pos;
layout(location = 1) in vec2 _texCoord;

out vec2 texCoord;

void main() {
    texCoord = _texCoord;
    gl_Position = vec4(_pos, 1.0f);
}
