#version 330 core

uniform sampler2D coarseTex;
uniform sampler2DMS depthTex;


out vec3 color;

const int radius = 14;
const int step = 2;
const float exaggeration = 1;

float linearStep(float start, float end, float x) {
    return clamp((x - start) / (end - start), 0, 1);
}

ivec2 center;
vec4 coarse;
float centerCoc;

vec4 getSample(ivec2 offset, float i) {
    vec4 current = texelFetch(
        coarseTex, center + offset, 0
    );
    float coc = current.a;
    float blurriness = abs(min(coc, centerCoc));
    float weight = 1 / (blurriness + 0.01);

    float stepCoc = radius * blurriness + 1;

    // anti-aliasing
    weight *= linearStep(stepCoc, stepCoc - 1, abs(i));

    return mix(
        vec4(0),
        vec4(current.rgb * weight, weight),
        bvec4(abs(i) <= stepCoc)
    );
}

void main() {
    center = ivec2(gl_FragCoord.xy);

    coarse = texelFetch(coarseTex, center, 0);
    centerCoc = coarse.a;

    vec4 colorA = vec4(0);
    vec4 colorB = vec4(0);

    for (int i = -radius; i <= radius; i++) {
        colorA += getSample(ivec2(i * step / 2, i * step), i);
        colorB += getSample(ivec2(-i * step / 2, i * step), i);
    }

    color = pow(
        min(colorA.rgb / colorA.a, colorB.rgb / colorB.a),
        vec3(1 / exaggeration)
    );
}
