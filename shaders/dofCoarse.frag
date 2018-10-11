#version 400 core

uniform sampler2DMS colorTex;
uniform sampler2DMS depthTex;
uniform sampler2D colorFilteredTex;
uniform sampler2D cocTex;

out vec4 coarse;

const int radius = 31;
const int step = 1;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);
    float centerDepth = texelFetch(depthTex, center, 0).r;

    coarse = vec4(
        texelFetch(colorFilteredTex, center, 0).rgb,
        texelFetch(cocTex, center, 0).r
    );
    int count = 1;

    for (int x = -radius; x <= radius; x += step) {
        float coc = texelFetch(cocTex, center + ivec2(x, 0), 0).r;

        if (abs(x) < radius * coc) {
            coarse += vec4(
                texelFetch(
                    colorTex, center + ivec2(x, 0), 0
                ).rgb,
                coc
            );
            count++;
        }
	}

    coarse = coarse / count;
}
