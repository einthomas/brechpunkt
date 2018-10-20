#version 330 core

#define M_PI 3.141592653589

uniform sampler2D gColorTex;
uniform sampler2DMS gNormalTex;
uniform sampler2DMS gWorldPosTex;
uniform sampler2D noiseTex;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 hemisphereSamples[64];

in vec2 texCoord;

out vec4 color;

void main() {
    const int numSamples = 32;
    const int stepSize = 1;
    const float strength = 8.0f;
    const float radius = 4.0f;

    vec3 normal = normalize(texelFetch(gNormalTex, ivec2(gl_FragCoord.xy * 2), 0).xyz);
    vec3 worldPos = texelFetch(gWorldPosTex, ivec2(gl_FragCoord.xy * 2), 0).xyz;

    vec3 lightPos = (view * vec4(0.0f, 1.0f, -10.0f, 1.0f)).xyz;
    vec3 lightDir = normalize(lightPos - worldPos);

    vec2 noiseStep = vec2(1280.0f, 720.0f) / 4.0f;
    vec3 rotationValue = texture(noiseTex, texCoord * noiseStep).xyz;

    vec3 right = normalize(rotationValue - normal * dot(rotationValue, normal));
    vec3 forward = cross(normal, right);
    mat3 basis = mat3(right, forward, normal);

    vec3 lightBounce = vec3(0.0f);
    for (int i = 0; i < numSamples; i += stepSize) {
        vec3 hemisphereSample = basis * hemisphereSamples[i];
        vec3 samplePos = worldPos + hemisphereSample * radius;
        
        vec4 samplePosImageSpace = projection * vec4(samplePos, 1.0f);
        samplePosImageSpace.xy /= samplePosImageSpace.w;
        samplePosImageSpace.xy = samplePosImageSpace.xy * 0.5f + 0.5f;

        vec4 sampleProjected = texelFetch(gWorldPosTex, ivec2(samplePosImageSpace.xy * vec2(1280.0f, 720.0f)), 0);

        float visibility = 1.0f;
        if (samplePosImageSpace.x < 0.0f ||
            samplePosImageSpace.x > 1.0f || samplePosImageSpace.y > 1.0f ||
            samplePosImageSpace.y < 0.0f || sampleProjected.w == 0.0f)
        {
            visibility = 1.0f;
        } else {
            visibility = sampleProjected.z <= samplePos.z + 0.03 ? 1.0f : smoothstep(0.0f, 2.0f, abs(sampleProjected.z - samplePos.z));
        }
        vec3 sampleNormal = normalize(texelFetch(gNormalTex, ivec2(samplePosImageSpace.xy * vec2(1280.0f, 720.0f)), 0).xyz);
        lightBounce += strength * max(0.0f, dot(sampleNormal, lightDir)) * (1.0f - visibility) * texture(gColorTex, samplePosImageSpace.xy).xyz;
    }

    color = vec4(pow(lightBounce / (numSamples / stepSize), vec3(2.2f)), 1.0f);
    color = vec4(0.0f);
}
