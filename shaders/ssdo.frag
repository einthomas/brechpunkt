#version 430

#define M_PI 3.141592653589

/*
uniform sampler2D gColorTex;
uniform sampler2D gNormalTex;
uniform sampler2D gWorldPosTex;
uniform sampler2D noiseTex;
uniform mat4 view;
uniform mat4 projection;
uniform float randomValues[64];
*/

in vec2 texCoord;

out vec4 color;

// generated using the method from "Sampling with Hammersley and Halton Points" [Wong et al., 1997]
// with parameters p1 = 2, p2 = 7
/*
const vec2 halton[64] = {
  vec2(0, 0),
  vec2(0.5, 0.142857),
  vec2(0.25, 0.285714),
  vec2(0.75, 0.428571),
  vec2(0.125, 0.571429),
  vec2(0.625, 0.714286),
  vec2(0.375, 0.857143),
  vec2(0.875, 0.0204082),
  vec2(0.0625, 0.163265),
  vec2(0.5625, 0.306122),
  vec2(0.3125, 0.44898),
  vec2(0.8125, 0.591837),
  vec2(0.1875, 0.734694),
  vec2(0.6875, 0.877551),
  vec2(0.4375, 0.0408163),
  vec2(0.9375, 0.183673),
  vec2(0.03125, 0.326531),
  vec2(0.53125, 0.469388),
  vec2(0.28125, 0.612245),
  vec2(0.78125, 0.755102),
  vec2(0.15625, 0.897959),
  vec2(0.65625, 0.0612245),
  vec2(0.40625, 0.204082),
  vec2(0.90625, 0.346939),
  vec2(0.09375, 0.489796),
  vec2(0.59375, 0.632653),
  vec2(0.34375, 0.77551),
  vec2(0.84375, 0.918367),
  vec2(0.21875, 0.0816327),
  vec2(0.71875, 0.22449),
  vec2(0.46875, 0.367347),
  vec2(0.96875, 0.510204),
  vec2(0.015625, 0.653061),
  vec2(0.515625, 0.795918),
  vec2(0.265625, 0.938776),
  vec2(0.765625, 0.102041),
  vec2(0.140625, 0.244898),
  vec2(0.640625, 0.387755),
  vec2(0.390625, 0.530612),
  vec2(0.890625, 0.673469),
  vec2(0.078125, 0.816327),
  vec2(0.578125, 0.959184),
  vec2(0.328125, 0.122449),
  vec2(0.828125, 0.265306),
  vec2(0.203125, 0.408163),
  vec2(0.703125, 0.55102),
  vec2(0.453125, 0.693878),
  vec2(0.953125, 0.836735),
  vec2(0.046875, 0.979592),
  vec2(0.546875, 0.00291545),
  vec2(0.296875, 0.145773),
  vec2(0.796875, 0.28863),
  vec2(0.171875, 0.431487),
  vec2(0.671875, 0.574344),
  vec2(0.421875, 0.717201),
  vec2(0.921875, 0.860058),
  vec2(0.109375, 0.0233236),
  vec2(0.609375, 0.166181),
  vec2(0.359375, 0.309038),
  vec2(0.859375, 0.451895),
  vec2(0.234375, 0.594752),
  vec2(0.734375, 0.737609),
  vec2(0.484375, 0.880467),
  vec2(0.984375, 0.0437318)
};

vec3 getHemisphereSample(vec2 u) {
    // source: "Physically Based Rendering: From Theory to Implementation" [Pharr and Humphreys, 2016]
    float r = sqrt(u.x);
    float phi = 2.0 * M_PI * u.y;
    return vec3(
        r * cos(phi),
        r * sin(phi),
        sqrt(max(0.0, 1.0 - u.x))
    );
}
*/

void main() {
    color = vec4(0.0f, 1.0f, 0.0f, 1.0f);


    //color = vec4(texture(gColorTex, texCoord).xyz, 1.0f);
    /*return;
    vec3 normal = normalize(texture(gNormalTex, texCoord).xyz);
    vec3 worldPos = texture(gWorldPosTex, texCoord).xyz;
    const int numSamples = 64;
    const float radius = 2.0f;

    vec3 lightPos = (view * vec4(-20.0f, 5.0f, 20.0f, 1.0f)).xyz;
    vec3 lightDir = normalize(lightPos - worldPos);


    vec3 up = vec3(0.01f, 0.99f, 0.0f);
    vec3 right = normalize(cross(up, normal));
    vec3 forward = normalize(cross(right, normal));
    mat3 basis = mat3(right, forward, normal);

    vec2 noiseStep = vec2(1280.0f, 720.0f) / 8.0f;
    vec3 c = vec3(0.0f);
    vec3 lightBounce = vec3(0.0f);
    //vec2 rv = texture(noiseTex, texCoord * noiseStep).xy;
    int index = int(texCoord.x * 8) + 8 * int(texCoord.y * 8);
    vec2 rvv = vec2(randomValues[index], randomValues[index + 1]);
    for (int i = 0; i < numSamples; i++) {
        vec3 sample_ = basis * getHemisphereSample(fract(halton[i]));// + rv));
        vec3 samplePos = worldPos + sample_ * radius * randomValues[i];
        
        vec4 samplePosImageSpace = projection * vec4(samplePos, 1.0f);
        samplePosImageSpace.xy /= samplePosImageSpace.w;
        samplePosImageSpace.xy = samplePosImageSpace.xy * 0.5f + 0.5f;

        float sampleDepth = texture(gWorldPosTex, samplePosImageSpace.xy).z;

        float strength = 1.0f;
        float visibility = 1.0f - strength * (sampleDepth > samplePos.z ? 1.0f : 0.0f) * smoothstep(3.0f, 0.0f, abs(sampleDepth - samplePos.z));
        if (samplePosImageSpace.x < 0.0f ||
            samplePosImageSpace.x > 1.0f || samplePosImageSpace.y > 1.0f ||
            samplePosImageSpace.y < 0.0f)
        {
            visibility = 1.0f;
        }
        c += max(0.0f, dot(normalize(samplePos - worldPos), lightDir)) * visibility * 3.0f;

            vec3 occluderColor = texture(gColorTex, samplePosImageSpace.xy).xyz;
            float lightBounceStrength = 4.0f;
            float d = min(1.0f, length(worldPos - samplePos));
            vec3 senderNormal = texture(gNormalTex, samplePosImageSpace.xy).xyz;
            vec3 transmittanceDir = normalize(worldPos - samplePos);
            lightBounce += occluderColor * (1.0f - visibility) * ((lightBounceStrength * max(0.0f, dot(senderNormal, transmittanceDir)) *
                max(0.0f, dot(normal, -transmittanceDir))) / d);
    }

    color = vec4(max(0.0f, dot(lightDir, mat3(view) * vec3(0.0f,1.0f,0.0f))) * vec3(1.0f), 1.0f);

    color = vec4((c + lightBounce) / numSamples, 1.0f);

    color = vec4(max(0.0f, dot(normal, lightDir)) * vec3(1.0f), 1.0f);

    color = vec4(texture(gColorTex, texCoord).xyz, 1.0f);
    */
}
