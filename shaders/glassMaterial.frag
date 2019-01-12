#version 330 core

layout(location = 0) out vec4 backfaceRefractionOut;
layout(location = 1) out vec4 backfacePosOut;

uniform sampler2D ssdoTex;
uniform sampler2DMS gWorldPosTex;
uniform sampler2DMS gNormalTex;

uniform sampler2DMS gWorldPosRefractiveTex;
uniform sampler2DMS gNormalRefractiveTex;
uniform sampler2DMS gOppositePosTex;
uniform sampler2DMS gRefractionTex;

uniform sampler2DMS gWorldPosLayer2Tex;
uniform sampler2DMS gNormalLayer2Tex;

uniform mat4 projection;
uniform vec2 size;

in vec2 texCoord;

void main() {
    float refractionFactor = texelFetch(gRefractionTex, ivec2(gl_FragCoord.xy), 0).x;
    if (refractionFactor > 0.0f) {
        vec3 worldPos = texelFetch(gWorldPosRefractiveTex, ivec2(gl_FragCoord.xy), 0).xyz;
        vec3 normal = texelFetch(gNormalRefractiveTex, ivec2(gl_FragCoord.xy), 0).xyz;

        vec3 refractionVector = normalize(refract(normalize(worldPos), normal, 1.5f));
        float depthLayer2 = texelFetch(gWorldPosLayer2Tex, ivec2(gl_FragCoord.xy), 0).z;
        float oppositeVertexDistance = length(
            texelFetch(gOppositePosTex, ivec2(gl_FragCoord.xy), 0).xyz - worldPos
        );

        float incidentAngle = dot(-normal, refractionVector);
        float refractionAngle = dot(normalize(worldPos), refractionVector);
        float f = refractionAngle / incidentAngle;
        float backfaceHitDistance = mix(oppositeVertexDistance, depthLayer2, f);
        vec3 backfaceHitPoint = worldPos + backfaceHitDistance * refractionVector;

        vec4 hitPointImageSpace = projection * vec4(backfaceHitPoint, 1.0f);
        hitPointImageSpace.xyz /= hitPointImageSpace.w;
        hitPointImageSpace.xyz = hitPointImageSpace.xyz * 0.5f + 0.5f;

        vec3 backfaceNormal = texelFetch(gNormalLayer2Tex, ivec2(hitPointImageSpace.xy * size), 0).xyz;
        vec3 backfaceRefraction = normalize(refract(refractionVector, -backfaceNormal, 0.67f));

        backfaceRefractionOut = vec4(backfaceRefraction, 1.0f);
        backfacePosOut = vec4(backfaceHitPoint, 1.0f);
    } else {
        discard;
    }
}
