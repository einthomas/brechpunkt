#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Mesh.h"

class ParticleSystem {
public:
    ParticleSystem(
        unsigned int capacity,
        GLint positionAttribute, GLint orientationAttribute
    );

    void draw(Program &shader);
    void add(glm::vec3 position, glm::quat orientation = {});

    Mesh mesh;
    GLuint instanceVbo;
    GLuint physicVbo;
    unsigned int particleCount;
    unsigned int capacity;
    unsigned int triangleCount;
};
