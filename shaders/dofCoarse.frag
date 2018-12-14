#version 330 core

uniform sampler2D colorTex;
uniform sampler2D cocTex;

out vec4 coarse;

const int radius = 16;
const int step = 1;
const float exaggeration = 1;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);

    float centerCoc = texelFetch(cocTex, center, 0).r;

    coarse = vec4(0);
    float count = 0;

    for (int i = -radius; i <= radius; i++) {
        ivec2 offset = ivec2(i * step, 0);
        vec3 current = pow(texelFetch(
            colorTex, center + offset, 0
        ).rgb, vec3(exaggeration));
        float coc = texelFetch(cocTex, center + offset, 0).r;
        float blurriness = abs(min(coc, centerCoc));
        float weight = 1 / (blurriness + 0.01);

        float stepCoc = radius * blurriness + 1;

        // anti-aliasing
        weight *= clamp(stepCoc - abs(i), 0, 1);

        bool mask = abs(i) <= stepCoc;

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
