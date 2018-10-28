#version 330 core

layout(location = 0) out vec4 colorOut;
layout(location = 1) out vec4 positionOut;
layout(location = 2) out vec4 normalOut;
layout(location = 3) out vec4 reflectionOut;
layout(location = 4) out vec3 emissionOut;

struct PointLight {
    vec3 pos;
    vec3 color;
    float constantTerm;
    float linearTerm;
    float quadraticTerm;
};
const int NUM_LIGHTS = 36;

uniform PointLight pointLights[NUM_LIGHTS];
uniform sampler2D diffuseTex;
uniform sampler2D reflectionTex;
uniform sampler2D normalTex;
uniform sampler2D noiseTex;
uniform bool useDiffuseTex;
uniform bool useNormalTex;
uniform vec3 diffuseColor;
uniform vec3 emissionColor;
uniform vec2 size;

uniform mat4 view;
uniform mat4 model;
//uniform vec3 color;

in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

void main() {    
    if (useNormalTex) {
        vec3 tangent = normalize(dFdx(worldPos));
        vec3 bitangent = normalize(cross(normal, tangent)); //normalize(dFdy(worldPos));
        mat3 tbn = mat3(tangent, bitangent, normalize(normal));

        vec2 noiseStep = size / 4.0f;
        vec3 noise = texture(noiseTex, texCoord * noiseStep).xyz;

        vec3 textureNormal = texture(normalTex, texCoord).xyz * 2.0 - 1.0;

        normalOut = vec4(normalize(tbn * textureNormal), 1.0f);
    } else {
        normalOut = vec4(normal, 1.0f);
    }

    vec3 diffuseColor_ = diffuseColor;
    if (useDiffuseTex) {
		diffuseColor_ = texture(diffuseTex, texCoord).xyz;
    }
    vec3 c = vec3(0.0f);
    for (int i = 0; i < NUM_LIGHTS; i++) {
        vec3 lightDir = pointLights[i].pos - worldPos;
        float lightDist = length(lightDir);
        float attenuation = 1.0f / (
            pointLights[i].constantTerm +
            pointLights[i].linearTerm * lightDist +
            pointLights[i].quadraticTerm * lightDist * lightDist
        );
        c += max(0.0f, dot(normalOut.xyz, normalize(lightDir))) * pointLights[i].color * attenuation * 50.0f * diffuseColor_;
    }
    c /= NUM_LIGHTS;
    c += emissionColor;
    
    colorOut = vec4(c, 1.0f);
    //colorOut = normalOut;

    positionOut = vec4(worldPos, 1.0f);
    
	reflectionOut = vec4(texture(reflectionTex, texCoord).xyz, 1.0f);
}
