#version 330 core

uniform sampler2D gColorTex;
uniform sampler2DMS gNormalTex;
uniform sampler2DMS gWorldPosTex;

uniform mat4 projection;
uniform vec2 size;

in vec2 texCoord;

out vec4 color;

vec2 binarySearch(vec3 pos, vec3 dir) {
    vec2 uv;
    for (int i = 0; i < 6; i++) {
        vec4 samplePosImageSpace = projection * vec4(pos, 1.0f);
        samplePosImageSpace.xy /= samplePosImageSpace.w;
        uv = samplePosImageSpace.xy * 0.5f + 0.5f;
        dir *= 0.5f;
        float depth = texelFetch(gWorldPosTex, ivec2(uv * size), 0).z;
        if (pos.z < depth) {
            pos -= dir;
        } else {
            pos += dir;
        }
    }
    return uv;
}

void main() {
    vec3 normal = normalize(texelFetch(gNormalTex, ivec2(gl_FragCoord.xy), 0).xyz);
    vec3 worldPos = texelFetch(gWorldPosTex, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 viewDir = normalize(worldPos);
    vec3 reflectionDir = normalize(reflect(viewDir, normal));

    vec3 stepDir = reflectionDir * 2.0f;
    vec3 samplePos = worldPos;
    bool hit = false;
    vec3 hitColor;
    for (int i = 0; i < 128; i++) {
        samplePos += stepDir;

        vec4 samplePosImageSpace = projection * vec4(samplePos, 1.0f);
        samplePosImageSpace.xy /= samplePosImageSpace.w;
        samplePosImageSpace.xy = samplePosImageSpace.xy * 0.5f + 0.5f;

        float depth = texelFetch(gWorldPosTex, ivec2(samplePosImageSpace.xy * size), 0).z;
        if (samplePos.z < depth) {
            hit = true;
            vec2 uv = binarySearch(samplePos, reflectionDir);
            hitColor = texture(gColorTex, uv).xyz;
            break;
        }
    }

    color = mix(vec4(1.0f), vec4(hitColor, 1.0f), hit);
}
