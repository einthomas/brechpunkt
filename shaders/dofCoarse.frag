#version 330 core

uniform sampler2D colorTex;
uniform sampler2D cocTex;

out vec4 coarse;

const int radius = 31;
const int step = 2;
const float exaggeration = 1;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);

    float centerCoc = texelFetch(cocTex, center, 0).r;

    float centerWeight = 1 / (abs(centerCoc) + 0.01);

    coarse = vec4(
        pow(texelFetch(colorTex, center, 0).rgb, vec3(exaggeration)),
        centerCoc
    ) * centerWeight;
    float count = centerWeight;

    for (int x = -radius; x <= radius; x += step) {
        ivec2 offset = ivec2(x, 0);
        vec3 current = pow(texelFetch(
            colorTex, center + offset, 0
        ).rgb, vec3(exaggeration));
        float coc = texelFetch(cocTex, center + offset, 0).r;
        float blurriness = abs(min(coc, centerCoc));
        float weight = 1 / (blurriness + 0.01);

        bool mask = abs(x) <= radius * blurriness;

        coarse += mix(
            vec4(0),
            vec4(current, blurriness) * weight,
            bvec4(mask)
        );

        count += mix(0, weight, mask);
    }

    coarse = coarse / count;
    coarse.a *= sign(centerCoc);
}
