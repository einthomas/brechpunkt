#version 330 core

uniform sampler2D coarseTex;
uniform sampler2DMS depthTex;

out vec3 color;

const int radius = 26; // 31 * sqrt(3) / 2
const int fineRadius = 2;
const int step = 2;
const float exaggeration = 1;

float linearStep(float start, float end, float x) {
    return clamp((x - start) / (end - start), 0, 1);
}

ivec2 center;
vec4 coarse;
float centerCoc;

vec4 getSample(ivec2 offset, float x) {
    vec4 current = texelFetch(
        coarseTex, center + offset, 0
    );
    float coc = current.a;
    float blurriness = abs(min(coc, centerCoc));
    float weight = 1 / (blurriness + 0.01);

    float pixelCoc = radius * blurriness + 1;

    // anti-aliasing
    weight *= linearStep(pixelCoc, pixelCoc - 1, abs(x));

    return mix(
        vec4(0),
        vec4(current.rgb * weight, weight),
        bvec4(abs(x) <= pixelCoc)
    );
}

void main() {
    center = ivec2(gl_FragCoord.xy);

    coarse = texelFetch(coarseTex, center, 0);
    centerCoc = coarse.a;

    vec4 colorA = vec4(0);
    vec4 colorB = vec4(0);

    for (int x = -radius; x <= radius; x += step) {
        colorA += getSample(ivec2(x * 0.5, x), x);
        colorB += getSample(ivec2(x * -0.5, x), x);
    }

    color = pow(
        min(colorA.rgb / colorA.a, colorB.rgb / colorB.a),
        vec3(1 / exaggeration)
    );
}
