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

vec3 rotate_vector(vec4 quat, vec3 vec) {
    return vec + 2.0 * cross(cross(vec, quat.xyz) + quat.w * vec, quat.xyz);
}

void main() {
    viewPosition = vec3(
        view *
        vec4(
            rotate_vector(_orientation, _vertexPosition * 0.1) +
            _position, 1.0f
        )
    );
    
    normal =
        transpose(inverse(mat3(view))) * rotate_vector(_orientation, _normal);
    
    texCoord = _texCoord;
    gl_Position = projection * vec4(viewPosition, 1.0f);
}
