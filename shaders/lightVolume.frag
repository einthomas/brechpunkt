#version 330 core

layout(location = 0) out float light;

uniform vec3 lightViewPos;

in vec3 viewPos;

const float density = 0.1;
const float d = 1 - density;
const float log_d = log(d);
const float log2_d = log_d * log_d;

float integral(float x, float a, float y) {
    // approximation of integral of
    // d^x * (1-d) * d^sqrt((x-a)^2+y^2) / ((x-a)^2+y^2) dx
    float y2 = y * y;
    return (
        atan(
            (y2 * a * log2_d + y * log_d * (-a + y) + 2.0 * a) /
            (y * sqrt(y2 * log2_d - 2.0 * y * log_d + 4.0))
        ) -
        atan(
            (y2 * (a - x) * log2_d + y * log_d * (-a + x + y) + 2.0 * (a - x)) /
            (y * sqrt(y2 * log2_d - 2.0 * y * log_d + 4.0))
        )
    ) * density * 2.0 * pow(d, (a + y)) / (
        y * sqrt(y2 * log2_d - 2.0 * y * log_d + 4.0)
    );
}

void main(void) {
    vec3 viewDirection = normalize(viewPos);

    float distance = length(viewPos);

    float lightDistance = dot(viewDirection, lightViewPos);
    float offset =
        max(length(lightViewPos - viewDirection * lightDistance), 0.1f);

    light = integral(distance, lightDistance, offset) * 100.0;

    light = mix(light, -light, gl_FrontFacing);
}
