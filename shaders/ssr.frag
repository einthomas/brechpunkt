#version 330 core

uniform sampler2D gColorTex;
uniform sampler2DMS gNormalTex;
uniform sampler2DMS gWorldPosTex;
uniform sampler2DMS gReflectionTex;
uniform samplerCube environmentColor;
uniform sampler2DMS depthTex;

uniform mat4 view;
uniform mat4 projection;
uniform vec2 size;

in vec2 texCoord;

out vec4 color;

vec3 worldToScreenSpace(vec3 worldPos) {
    vec4 screenSpacePos = projection * vec4(worldPos, 1.0f);
    screenSpacePos /= screenSpacePos.w;
    screenSpacePos.xy = screenSpacePos.xy * 0.5f + 0.5f;
    return screenSpacePos.xyz;
}

const float MARCH_STEP_SIZE = 1.0f;

void main() {
    vec3 reflectionFactor = texelFetch(
        gReflectionTex, ivec2(gl_FragCoord.xy), 0
    ).xyz;

    vec3 worldPos = texelFetch(gWorldPosTex, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 normal = normalize(
        texelFetch(gNormalTex, ivec2(gl_FragCoord.xy), 0).xyz
    );
    vec3 reflectionDir = normalize(reflect(normalize(worldPos), normal));

    vec3 hitColor = vec3(0.0f);
    vec3 marchStep = reflectionDir * MARCH_STEP_SIZE;
    vec3 samplePos = worldPos;
    for (int i = 0; i < 32; i++) {
        samplePos += marchStep;
        vec3 screenSpacePos = worldToScreenSpace(samplePos);

        float depth = texelFetch(
            gWorldPosTex, ivec2(screenSpacePos.xy * size), 0
        ).z;
        if (samplePos.z < depth) {
            // binary search
            for (int k = 0; k < 6; k++) {
                marchStep *= 0.5f;
                screenSpacePos = worldToScreenSpace(samplePos);

                depth = texelFetch(
                    gWorldPosTex, ivec2(screenSpacePos.xy * size), 0
                ).z;
                if (samplePos.z < depth) {
                    samplePos += marchStep;
                } else {
                    samplePos -= marchStep;
                }
            }

            if (screenSpacePos.x >= 0.0f && screenSpacePos.x <= 1.0f &&
                screenSpacePos.y >= 0.0f && screenSpacePos.y <= 1.0f)
            {
                hitColor = texture(gColorTex, screenSpacePos.xy).xyz;
            } else {
                hitColor = texture(environmentColor, worldPos + reflectionDir).xyz;
            }
            break;
        }
    }

    color = vec4(hitColor * reflectionFactor, 1.0f);
}
