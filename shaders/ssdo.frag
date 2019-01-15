#version 330 core

#define M_PI 3.141592653589

const int NUM_SAMPLES = 3;
const float RADIUS = 0.5f;

uniform sampler2D noiseTex;
uniform samplerCube environmentColor;
uniform float environmentBrightness;

uniform sampler2D gColorTex;
uniform sampler2D gWorldPosTex;
uniform sampler2D gNormalTex;
uniform sampler2D gEmissionTex;
uniform sampler2D gDepthTex;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 inverseProjection;

in vec2 texCoord;

out vec4 color;

vec3 getHemisphereSample(vec2 u) {
    // source: "Physically Based Rendering: From Theory to Implementation"
    // [Pharr and Humphreys, 2016]
    float r = sqrt(u.x);
    float phi = 2.0f * 3.1415926f * u.y;
    return vec3(
        r * cos(phi),
        r * sin(phi),
        sqrt(max(0.0f, 1.0f - u.x))
    );
}

vec3 getViewPosition(vec3 ndc) {
    vec4 viewPosition = inverseProjection * vec4(ndc, 1);
    return viewPosition.xyz / viewPosition.w;
}

void main() {
    vec3 normal = texelFetch(gNormalTex, ivec2(gl_FragCoord.xy), 0).xyz;
    //vec3 worldPos = texelFetch(gWorldPosTex, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 worldPos = getViewPosition(
        vec3(texCoord, texelFetch(gDepthTex, ivec2(gl_FragCoord.xy), 0).r) *
        2 - 1
    );

    vec3 noise = texelFetch(noiseTex, ivec2(mod(gl_FragCoord.xy, 4)), 0).xyz;
    
    vec3 right = normalize(cross(vec3(0, 1, 0), normal));
    vec3 forward = cross(normal, right);
    mat3 basis = mat3(right, forward, normal);

    vec4 c = vec4(0.0f);
    for (int x = 0; x < NUM_SAMPLES; x++) {
        for (int y = 0; y < NUM_SAMPLES; y++) {
            vec3 hemisphereSample = basis * getHemisphereSample(
                (vec2(x, y) + noise.yz) / float(NUM_SAMPLES)
            ) * noise.z;

            vec3 sampleCubeMap = texture(
                environmentColor,
                transpose(mat3(view)) * hemisphereSample
            ).rgb;

            //if (any(greaterThan(sampleCubeMap, vec3(0.01)))) {
                vec3 samplePos = worldPos + hemisphereSample * RADIUS;
                vec4 samplePosImageSpace = projection * vec4(samplePos, 1.0f);
                samplePosImageSpace.xyz /= samplePosImageSpace.w;
                samplePosImageSpace.xyz = samplePosImageSpace.xyz * 0.5f + 0.5f;

                if (
                    samplePosImageSpace.x >= 0.0f &&
                    samplePosImageSpace.x <= 1.0f &&
                    samplePosImageSpace.y >= 0.0f &&
                    samplePosImageSpace.y <= 1.0f
                ) {
                    float sampleDepth = texelFetch(
                        gDepthTex,
                        ivec2(samplePosImageSpace.xy * textureSize(gDepthTex, 0)),
                        0
                    ).r;

                    c += vec4(
                        mix(
                            vec3(0.0),
                            sampleCubeMap, sampleDepth > samplePosImageSpace.z
                        ),
                        float(sampleDepth > samplePosImageSpace.z - 0.01)
                    ) * environmentBrightness;
                }
            //} else {
            //    c += vec4(0, 0, 0, 1);
            //}
        }
    }

    vec3 emissionColor = texelFetch(gEmissionTex, ivec2(gl_FragCoord.xy), 0).xyz;

    vec3 diffuseColor = texture(gColorTex, texCoord).xyz;
    color = vec4(c.rgb / max(c.a, 1) * diffuseColor + emissionColor, 1.0f);

    //color = vec4(
    //    texelFetch(noiseTex, ivec2(mod(gl_FragCoord.xy, 8)), 0).rgb, 1
    //);
    //color = vec4(texture(
    //    environmentColor,
    //    transpose(mat3(view)) * reflect(worldPos, normal)
    //).rgb, 1.0f);
}
