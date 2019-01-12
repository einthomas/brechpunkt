#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Mesh.h"

struct Particle {
    Particle() = default;
    Particle(glm::vec3 p, glm::quat o, glm::vec3 f);

    glm::vec3 position;
    float padding0;
    glm::quat orientation;
    glm::vec3 force;
    float padding1;
};

class ParticleSystem {
public:
    ParticleSystem(
        unsigned int capacity,
        GLint positionAttribute, GLint orientationAttribute,
        GLint forceAttribute
    );

    void draw(Program &shader);
    void add(glm::vec3 position, glm::quat orientation, glm::vec3 force);
    void add(Particle* begin, Particle* end);

    Mesh mesh;
    GLuint instanceVbo;
    GLuint physicVbo;
    unsigned int particleCount;
    unsigned int nextIndex;
    unsigned int capacity;
    unsigned int triangleCount;
};


inline Particle::Particle(glm::vec3 p, glm::quat o, glm::vec3 f) :
    position(p), orientation(o), force(f) {}
