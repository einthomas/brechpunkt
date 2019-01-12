#version 330 core

uniform sampler2D colorTex;
uniform sampler2D gNormalTex;
uniform sampler2D gDepthTex;
uniform mat4 inverseProjection;

in vec2 texCoord;

out vec4 color;

const int sigma = 4;
const int radius = 6;

vec3 getViewPosition(vec3 ndc) {
    vec4 viewPosition = inverseProjection * vec4(ndc, 1);
    return viewPosition.xyz / viewPosition.w;
}

float getDepth(ivec2 position) {
    return getViewPosition(
        vec3(0, 0, texelFetch(gDepthTex, position, 0).r * 2 - 1)
    ).z;
}

void main() {
    color = vec4(0);
    
    vec3 normal = texelFetch(gNormalTex, ivec2(gl_FragCoord.xy), 0).xyz;
    float depth = getDepth(ivec2(gl_FragCoord.xy));

	ivec2 center = ivec2(gl_FragCoord.xy);
    for (int i = -radius; i <= radius; i++) {
        float weight = exp(-float(i * i) / (2 * sigma * sigma));
        ivec2 uv = center + ivec2(i, 0);

        vec3 sampleNormal = texelFetch(gNormalTex, uv, 0).xyz;
        float sampleDepth = getDepth(uv);

        bool accept =
            dot(sampleNormal, normal) > 0.995 &&
            abs(depth - sampleDepth) < 0.5 || i == 0;

        if (accept) {
            color += vec4(
                texelFetch(colorTex, uv, 0).rgb * weight,
                weight
            );
        }
	}

	color = vec4(color.xyz / color.w, 1.0f);
}
