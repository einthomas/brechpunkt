#version 450 core

layout(location = 0) out vec3 color;

uniform sampler2D colorTex;
uniform sampler2DMS gDepthTex;
uniform sampler2DMS gNormalTex;

in vec3 viewPosition;
in vec3 normal;

void main() {
    float minDistance = 2;
    ivec2 offset = ivec2(0);

    vec4 target = vec4(normalize(normal), gl_FragCoord.z);
    // TODO: check normal texture!

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec4 current = vec4(
                texelFetch(
                    gNormalTex, ivec2(gl_FragCoord.xy) + ivec2(x, y), 0
                ).xyz,
                texelFetch(
                    gDepthTex, ivec2(gl_FragCoord.xy) + ivec2(x, y), 0
                ).r
            );

            vec4 delta = target - current;
            float distance = dot(delta.xyz, delta.xyz);

            bool closer = distance < minDistance;
            minDistance = mix(minDistance, distance, closer);
            offset.x = mix(offset.x, x, closer);
            offset.y = mix(offset.y, y, closer);
        }
    }

    color = texelFetch(colorTex, ivec2(gl_FragCoord.xy) + offset, 0).xyz;
}
