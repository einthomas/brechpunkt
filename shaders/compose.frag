#version 330 core

uniform sampler2D dofTex;
uniform sampler2D bloomTex;

uniform sampler2D noteTex;
uniform float noteAnimation;

in vec2 texCoord;

out vec4 color;

void main() {
    vec3 inColor = mix(
        texture(dofTex, texCoord).rgb, texture(bloomTex, texCoord).rgb, 0.1
    );

    vec2 uv = texCoord * vec2(16, 9) * vec2(0.25, -1) * 0.5 + vec2(-0.025, 1.1);
    uv += vec2(noteAnimation, 0);
    vec3 note = pow(texture(noteTex, uv).rgb, vec3(2.2f));

    inColor = mix(inColor, note, bvec3(
        all(greaterThan(uv, vec2(0))) && all(lessThan(uv, vec2(1)))
    ));

    color = vec4(pow(inColor, vec3(1.0f / 2.2f)), 1.0f);
}
