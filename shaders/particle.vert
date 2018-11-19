#version 330 core

layout(location = 0) in vec3 _vertexPosition;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec2 _texCoord;
layout(location = 3) in vec3 _position;
layout(location = 4) in vec4 _orientation;

uniform mat4 view;
uniform mat4 projection;
uniform bool useNormalTex;

out vec3 viewPosition;
out vec3 normal;
out vec2 texCoord;

void main() {
    viewPosition = vec3(view * vec4(_vertexPosition * 0.1 + _position, 1.0f));
    
    normal = transpose(inverse(mat3(view))) * _normal;
    
    texCoord = _texCoord;
    gl_Position = projection * vec4(viewPosition, 1.0f);
}
