#version 330 core

layout(location = 0) out vec4 colorOut;
layout(location = 1) out vec4 positionOut;
layout(location = 2) out vec4 normalOut;
layout(location = 3) out vec4 reflectionOut;

uniform sampler2D diffuseTex;
uniform sampler2D reflectionTex;
uniform sampler2D normalTex;
uniform bool useNormalTex;
uniform bool useDiffuseTex;

in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

void main() {
    vec3 color;
    if (useDiffuseTex) {
        color = texture(diffuseTex, texCoord).xyz;
    } else {
        color = vec3(5.0f);
    }
    colorOut = vec4(pow(color, vec3(2.2f)), 1.0f);

    positionOut = vec4(worldPos, 1.0f);

    if (useNormalTex) {
        normalOut = vec4(normalize(texture(normalTex, texCoord).xzy * 0.5f + 0.5f), 1.0f);
    } else {
        normalOut = vec4(normalize(normal * 0.5f + 0.5f), 1.0f);
    }

    reflectionOut = vec4(texture(reflectionTex, texCoord).xyz, 1.0f);
}
