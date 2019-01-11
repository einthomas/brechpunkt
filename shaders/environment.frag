#version 330 core

layout(location = 0) out vec3 colorOut;

uniform sampler2D diffuseTex;
uniform sampler2D reflectionTex;
uniform sampler2D normalTex;
uniform bool useNormalTex;
uniform bool useDiffuseTex;
uniform vec3 diffuseColor;
uniform vec3 emissionColor;
uniform float emissionColorBrightness;

in vec3 geometryNormal;
in vec2 geometryTexCoord;

void main() {
    vec3 color;
    if (useDiffuseTex) {
        color = texture(diffuseTex, geometryTexCoord).xyz;
    } else {
        color = diffuseColor;
    }

    colorOut = pow(color, vec3(2.2f)) + emissionColor * emissionColorBrightness;
}
