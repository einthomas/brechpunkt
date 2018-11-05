#pragma once

#include <GL/glew.h>

class ParticleSystem {
public:
    ParticleSystem(
        int capacity, GLint positionAttribute, GLint orientationAttribute
    );

    void draw();

    GLuint vao;
    GLuint objectVbo;
    GLuint instanceVbo;
    unsigned int particleCount;
    unsigned int triangleCount;
};
