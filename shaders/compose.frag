#version 330 core

uniform sampler2DMS colorTex;
uniform sampler2D bloomTex;

in vec2 texCoord;

out vec4 color;

void main() {
	vec3 inColor = vec3(0);

	for (int i = 0; i < 4; i++) {
        inColor += texelFetch(colorTex, ivec2(gl_FragCoord.xy), i).rgb;
	}

	inColor *= 0.25;

    inColor = mix(inColor, texture(bloomTex, texCoord).rgb, 0.05);

    color = vec4(pow(inColor, vec3(1.0f / 2.2f)), 1.0f);
}
