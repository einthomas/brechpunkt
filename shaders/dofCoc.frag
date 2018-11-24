#version 330 core

uniform sampler2DMS depthTex;
uniform float focus;

out float coc;

float depth(float x) {
	// stackoverflow.com/questions/6652253
	float z_b = x;
	float z_n = 2.0 * z_b - 1.0;

    // TODO: make these uniforms
    float near = 0.1, far = 100.0;

    return 2.0 * near * far / (far + near - z_n * (far - near));
}

float signedCoc(float depth) {
    return (depth - focus) / depth;
}

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);
    coc = signedCoc(depth(texelFetch(depthTex, center, 0).r));
}
