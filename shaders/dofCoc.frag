#version 330 core

uniform sampler2DMS depthTex;

out float coc;

const float aperture = 1;
const float focus = 8;

float depth(float x) {
	// stackoverflow.com/questions/6652253
	float z_b = x;
	float z_n = 2.0 * z_b - 1.0;

    // TODO: make these uniforms
    float near = 0.1, far = 100.0;

    return 2.0 * near * far / (far + near - z_n * (far - near));
}

float signedCoc(float depth) {
    return aperture * (depth - focus) / depth;
}

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);
    coc = abs(signedCoc(depth(texelFetch(depthTex, center, 0).r)));
}
