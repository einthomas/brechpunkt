#version 330 core

uniform sampler2D colorTex;
uniform sampler2DMS gNormalTex;

in vec2 texCoord;

out vec4 color;

const int sigma = 7;
const int radius = 9;
const int step = 1;

void main() {
    color = vec4(0, 0, 0, 1);
    
    vec3 normal = normalize(texelFetch(gNormalTex, ivec2(gl_FragCoord.xy), 0).xyz);
	ivec2 center = ivec2(gl_FragCoord.xy);
	for (int i = -radius; i <= radius; i += step) {
		float weight = exp(-float(i * i) / (2 * sigma * sigma));
        ivec2 uv = center + ivec2(i, 0);
        vec3 sampleNormal = normalize(texelFetch(gNormalTex, uv, 0).xyz);
        if (dot(sampleNormal, normal) > 0.99) {
            color += vec4(
                texelFetch(colorTex, uv, 0).rgb * weight,
                weight
            );
        }
	}

	color = vec4(color.xyz / color.w, 1.0f);
}
