#version 330 core

uniform sampler2D colorTex;

out vec4 color;

const int sigma = 7;
const int radius = 9;
const int step = 1;

void main() {
	color = vec4(0);

	ivec2 center = ivec2(gl_FragCoord.xy);

	for (int i = -radius; i <= radius; i += step) {
		float weight = exp(-float(i * i) / (2 * sigma * sigma));
		color += vec4(
			texelFetch(
				colorTex, center + ivec2(0, i), 0
			).rgb * weight,
			weight
		);
	}

	color = vec4(color.xyz / color.w, 1.0f);
}
