#version 330 core

uniform sampler2DMS colorTex;
uniform sampler2DMS depthTex;
uniform sampler2D colorFilteredTex;
uniform sampler2D cocTex;
uniform sampler2D noiseTex;

out vec4 coarse;

const int radius = 31;
const int step = 15;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);

    float centerCoc = texelFetch(cocTex, center, 0).r;
    float centerDepth = texelFetch(depthTex, center, 0).r;

    float centerWeight = 1 / (abs(centerCoc) + 0.01) / float(step) / float(step);

    coarse = vec4(
        texelFetch(colorFilteredTex, center, 0).rgb,
        1
    ) * centerWeight;

    ivec2 seed = ivec2(
        texelFetch(
            noiseTex, ivec2(mod(gl_FragCoord.xy, 4)), 0
        ).r * step,
        texelFetch(
            noiseTex, ivec2(mod(gl_FragCoord.xy + ivec2(2), 4)), 0
        ).r * step
    );

    for (int x = -radius; x <= radius; x += step) {
        for (int y = -radius; y <= radius; y += step) {
            ivec2 offset = ivec2(x, y) + seed;
            vec4 current = texelFetch(
                colorTex, center + offset, 0
            );
            float coc = texelFetch(cocTex, center + offset, 0).r;
            float depth = texelFetch(depthTex, center + offset, 0).r;
            float blurriness = mix(coc, centerCoc, centerDepth < depth);
            float weight = 1 / (blurriness + 0.01);

            coarse += mix(
                vec4(0),
                vec4(current.rgb, 1) * weight,
                bvec4(abs(offset.x) + abs(offset.y) <= radius * blurriness)
            );
        }
    }

    coarse = coarse / coarse.a;
}
