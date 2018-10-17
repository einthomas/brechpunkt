#version 400 core

uniform sampler2DMS colorTex;
uniform sampler2D colorFilteredTex;
uniform sampler2D cocTex;

out vec4 coarse;

const int radius = 31;
const int step = 1;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);
    float centerCoc = texelFetch(cocTex, center, 0).r;
    float centerWeight = 1 / (abs(centerCoc) + 0.01);

    coarse = vec4(
        texelFetch(colorFilteredTex, center, 0).rgb,
        texelFetch(cocTex, center, 0).r
    ) * centerWeight;
    float count = centerWeight;

    for (int x = -radius; x <= radius; x += step) {
        float coc = texelFetch(cocTex, center + ivec2(x, 0), 0).r;
        float blurriness = abs(min(coc, centerCoc));
        float weight = 1 / (blurriness + 0.01);

        if (abs(x) < radius * blurriness) {
            coarse += vec4(
                texelFetch(
                    colorTex, center + ivec2(x, 0), 0
                ).rgb,
                coc
            ) * weight;
            count += weight;
        }
    }

    coarse = coarse / count;
}
