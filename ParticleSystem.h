#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Mesh.h"

class ParticleSystem {
public:
    ParticleSystem(
        unsigned int capacity,
        GLint positionAttribute, GLint orientationAttribute,
        GLint forceAttribute
    );

    void draw(Program &shader);
    void add(glm::vec3 position, glm::quat orientation, glm::vec3 force);

    Mesh mesh;
    GLuint instanceVbo;
    GLuint physicVbo;
    unsigned int particleCount;
    unsigned int nextIndex;
    unsigned int capacity;
    unsigned int triangleCount;
};
