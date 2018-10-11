#version 330 core

uniform sampler2D tex;
uniform sampler2D normalTex;
uniform vec2 size;
uniform vec2 dir;
uniform int kernelSize;

in vec2 texCoord;

out vec4 color;

void main() {
    vec3 normal = normalize(texture(normalTex, texCoord).xyz);

    vec3 c = vec3(0.0f);

    float sigma = 5.0f;
    float f = 1.0f / (sqrt(2.0f * 3.1415f * sigma * sigma));
    float weightSum = 0.0f;
    vec2 textureStep = (vec2(1.0f) / size) * dir;
    for (int i = -kernelSize; i <= kernelSize; i++) {
        vec2 uv = texCoord - textureStep * i;
        vec3 sampleNormal = normalize(texture(normalTex, uv).xyz);
        if (dot(sampleNormal, normal) > 0.99) {
            float weight = f * exp(-pow(i / (2.0f * sigma * sigma), 2.0f));
            weightSum += weight;
            c += texture(tex, uv).xyz * weight;
        }
    }

    color = vec4(c / weightSum, 1.0f);
}
