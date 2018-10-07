#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D reflectionTex;
uniform sampler2D normalTex;
uniform bool useNormalTex;
uniform bool useDiffuseTex;

in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

out vec4 color;

void main() {
    vec3 c = texture(diffuseTex, texCoord).xyz;
    color = vec4(c, 1.0f);
}
