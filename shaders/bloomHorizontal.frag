#version 400 core

// TODO: resolve samples before
uniform sampler2DMS colorTex;

out vec4 color;

int sigma = 24;

void main() {
	color = vec4(0);

	for (int i = -63; i <= 63; i++) {
		float weight = exp(-float(i * i) / (2 * sigma * sigma));
		color += vec4(
			texelFetch(
				colorTex, ivec2(gl_FragCoord.x + i, gl_FragCoord.y), 0
			).rgb * weight,
			weight
		);
	}

	color = vec4(color.xyz / color.w, 1.0f);
}
