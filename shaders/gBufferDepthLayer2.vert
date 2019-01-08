#version 330 core

layout(location = 0) in vec3 _pos;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec2 _texCoord;
layout(location = 3) in vec3 _oppositePos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool useNormalTex;

out vec3 worldPos;
out vec3 normal;
out vec2 texCoord;

void main() {
    vec4 worldPos4 = view * model * vec4(_pos, 1.0f);
    worldPos = worldPos4.xyz;
    
    normal = transpose(inverse(mat3(view * model))) * _normal;
    
    texCoord = _texCoord;
    gl_Position = projection * worldPos4;
}
