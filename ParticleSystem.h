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

    void draw(Shader &shader);
    void add(glm::vec3 position);

    Mesh mesh;
    GLuint instanceVbo;
    unsigned int particleCount;
    unsigned int capacity;
    unsigned int triangleCount;
};
