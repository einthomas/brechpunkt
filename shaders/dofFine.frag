#version 400 core

uniform sampler2D coarseTex;

out vec3 color;

const int radius = 26; // 31 * sin(pi / 3)
const int fineRadius = 2;
const int step = 1;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);

    vec4 coarse = texelFetch(coarseTex, center, 0);
    float centerCoc = coarse.a;

    vec4 colorA = vec4(0);
    vec4 colorB = vec4(0);

    for (int x = -radius; x <= radius; x += step) {
        vec4 current = texelFetch(
            coarseTex, center + ivec2(x / 2, x), 0
        );
        float coc = current.a;
        float blurriness = abs(min(coc, centerCoc));
        float weight = 1 / (blurriness + 0.01);

        if (abs(x) <= radius * blurriness) {
            colorA += vec4(current.rgb * weight, weight);
        }

        current = texelFetch(
            coarseTex, center + ivec2(-x / 2, x), 0
        );
        coc = current.a;
        blurriness = abs(min(coc, centerCoc));
        weight = 1 / (blurriness + 0.01);

        if (abs(x) <= radius * blurriness) {
            colorB += vec4(current.rgb * weight, weight);
        }
    }

    color = min(colorA.rgb / colorA.a, colorB.rgb / colorB.a);
}
