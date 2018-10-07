#version 330 core

uniform sampler2D colorTex;

in vec2 texCoord;

out vec4 color;

void main() {
    color = vec4(texture(colorTex, texCoord).xyz, 1.0f);
}
