#version 400 core

uniform sampler2DMS colorTex;
uniform sampler2D bloomTex;
uniform sampler2D occlusionTex;

in vec2 texCoord;

out vec4 color;

void main() {
	vec3 inColor = texture(occlusionTex, texCoord).xyz * 0.25;
    inColor = mix(inColor, texture(bloomTex, texCoord).rgb, 0.05);

	color = vec4(pow(inColor, vec3(1.0f / 2.2f)), 1.0f);
}
