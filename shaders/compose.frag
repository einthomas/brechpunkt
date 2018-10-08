#version 400 core

uniform sampler2DMS colorTex;

in vec2 texCoord;

out vec4 color;

void main() {
	vec3 inColor = vec3(0);

	for (int i = 0; i < 4; i++) {
		inColor += texelFetch(colorTex, ivec2(gl_FragCoord.xy), i).xyz;
	}

	inColor *= 0.25;

	color = vec4(inColor, 1.0f);
}
