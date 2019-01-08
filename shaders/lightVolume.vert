#version 330 core

layout(location = 0) in vec3 _pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 viewPos;

void main(void) {
    viewPos = vec3(view * model * vec4(_pos, 1.0f));
    gl_Position = projection * vec4(viewPos, 1.0f);
}
