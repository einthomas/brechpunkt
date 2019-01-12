#version 330 core

layout(location = 0) in vec3 _pos;
layout(location = 1) in vec3 _normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 viewPosition;
out vec3 normal;

void main() {
    viewPosition = vec3(view * model * vec4(_pos, 1.0f));
    
    normal = transpose(inverse(mat3(view * model))) * _normal;
    
    gl_Position = projection * vec4(viewPosition, 1.0f);
}
