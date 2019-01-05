#version 330 core

uniform sampler2D dofTex;
uniform sampler2D bloomTex;

in vec2 texCoord;

out vec4 color;

void main() {
    vec3 inColor = mix(
        texture(dofTex, texCoord).rgb, texture(bloomTex, texCoord).rgb, 0.1
    );

    color = vec4(pow(inColor, vec3(1.0f / 2.2f)), 1.0f);
}
