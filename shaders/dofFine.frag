#version 400 core

uniform sampler2D coarseTex;

out vec3 color;

const int radius = 26; // 31 * sin(pi / 3)
const int fineRadius = 2;
const int step = 1;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);

    vec4 colorA = vec4(texelFetch(coarseTex, center, 0).rgb, 1);
    vec4 colorB = vec4(texelFetch(coarseTex, center, 0).rgb, 1);

    for (int x = -radius; x <= radius; x += step) {
        vec4 current = texelFetch(
            coarseTex, center + ivec2(x / 2, x), 0
        );
        float coc = current.a;

        if (abs(x) < radius * coc) {
            colorA += vec4(current.rgb, 1);
        }

        current = texelFetch(
            coarseTex, center + ivec2(-x / 2, x), 0
        );
        coc = current.a;

        if (abs(x) < radius * coc) {
            colorB += vec4(current.rgb, 1);
        }
    }

    color = min(colorA.rgb / colorA.a, colorB.rgb / colorB.a);
}
