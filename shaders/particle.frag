#version 330 core

layout(location = 0) out vec4 colorOut;
layout(location = 1) out vec4 positionOut;
layout(location = 2) out vec4 normalOut;
layout(location = 3) out vec4 reflectionOut;
layout(location = 4) out vec3 emissionOut;
layout(location = 5) out float primitiveIDOut;

struct PointLight {
    vec3 pos;
    vec3 color;
    float brightness;
    float constantTerm;
    float linearTerm;
    float quadraticTerm;
};
const int NUM_LIGHTS = 36;

uniform PointLight pointLights[NUM_LIGHTS];

uniform mat4 view;

in vec3 viewPosition;
in vec3 normal;

void main() {
    normalOut = vec4(normalize(normal), 1.0f);

    vec3 viewDirection = normalize(viewPosition);

    vec3 emissive = vec3(0.0f);
    for (int i = 0; i < NUM_LIGHTS; i++) {
        vec3 lightDir = pointLights[i].pos - viewPosition;
        float lightDist = length(lightDir);
        lightDir /= lightDist;

        emissive += pointLights[i].brightness * max(
            0.0f,
            pow(
                dot(
                    reflect(lightDir, normalOut.xyz),
                    viewDirection
                ),
                100
            )
        )  * pointLights[i].color / lightDist / lightDist * 1000;
    }
    emissionOut = emissive;
    
    vec3 diffuseColor_ = vec3(0);
    colorOut = vec4(diffuseColor_, 1.0f);

    positionOut = vec4(viewPosition, 1.0f);

    primitiveIDOut = mod(gl_PrimitiveID, 256) / 255.0f;
}
