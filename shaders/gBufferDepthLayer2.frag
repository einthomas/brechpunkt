#version 330 core

layout(location = 0) out vec4 positionOut;
layout(location = 1) out vec4 normalOut;

uniform sampler2D normalTex;
uniform sampler2DMS depthLayer1Tex;
uniform bool useNormalTex;

uniform mat4 view;
uniform mat4 model;

in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

void main() {
    float depthLayer1 = texelFetch(depthLayer1Tex, ivec2(gl_FragCoord.xy), 0).x;

    if (useNormalTex) {
        vec3 tangent = normalize(dFdx(worldPos));
        vec3 bitangent = normalize(cross(normal, tangent)); //normalize(dFdy(worldPos));
        mat3 tbn = mat3(tangent, bitangent, normalize(normal));

        vec3 textureNormal = texture(normalTex, texCoord).xyz * 2.0 - 1.0;

        normalOut = vec4(normalize(tbn * textureNormal), 1.0f);
    } else {
        normalOut = vec4(normal, 1.0f);
    }

    positionOut = vec4(worldPos, 1.0f);
}
