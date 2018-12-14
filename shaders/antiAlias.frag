#version 450 core

layout(location = 0) out vec3 color;

uniform sampler2D colorTex;
uniform sampler2D gPrimitiveIDTex;

in vec3 viewPosition;
in vec3 normal;

ivec2 position;

void gather(ivec2 offset) {
    color = mix(
        color,
        texelFetch(colorTex, position + offset, 0).rgb,
        round(texelFetch(gPrimitiveIDTex, position + offset, 0).r * 255) ==
        mod(gl_PrimitiveID, 256)
    );
}

void main() {
    position = ivec2(gl_FragCoord.xy);

    vec3 center = texelFetch(colorTex, position, 0).rgb;
    color = center;

    gather(ivec2(-1, 0));
    gather(ivec2(1, 0));
    gather(ivec2(0, -1));
    gather(ivec2(0, 1));

    color = mix(
        color,
        center,
        round(texelFetch(gPrimitiveIDTex, position, 0).r * 255) ==
        mod(gl_PrimitiveID, 256)
    );
}
