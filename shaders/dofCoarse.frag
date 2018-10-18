#version 330 core

uniform sampler2DMS colorTex;
uniform sampler2DMS depthTex;
uniform sampler2D colorFilteredTex;
uniform sampler2D cocTex;

out vec4 coarse;

const int radius = 31;
const int step = 1;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);

    float centerCoc = texelFetch(cocTex, center, 0).r;
    float centerDepth = texelFetch(depthTex, center, 0).r;

    float centerWeight = 1 / (abs(centerCoc) + 0.01);

    coarse = vec4(
        texelFetch(colorFilteredTex, center, 0).rgb,
        centerCoc
    ) * centerWeight;
    float count = centerWeight;

    for (int x = -radius; x <= radius; x += step) {
        ivec2 offset = ivec2(x, 0);
        vec4 current = texelFetch(
            colorTex, center + offset, 0
        );
        float coc = texelFetch(cocTex, center + offset, 0).r;
        float depth = texelFetch(depthTex, center + offset, 0).r;
        float blurriness = mix(coc, centerCoc, centerDepth < depth);
        float weight = 1 / (blurriness + 0.01);

        bool mask = abs(x) <= radius * blurriness;

        coarse += mix(
            vec4(0),
            vec4(current.rgb, coc) * weight,
            bvec4(mask)
        );

        count += mix(0, weight, mask);
    }

    coarse = coarse / count;
}
