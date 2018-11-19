#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(
    unsigned int capacity, GLint positionAttribute, GLint orientationAttribute
) : capacity(capacity) {
    mesh = Mesh(
        {"scenes/scene3/", "Particle.obj"}, glm::mat4(1), {1, 1, 1}, {0, 0, 0}
    );

    glGenBuffers(1, &instanceVbo);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glBufferData(
        GL_ARRAY_BUFFER, capacity * (3 + 4) * sizeof(float),
        nullptr, GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        positionAttribute, 3, GL_FLOAT, GL_FALSE,
        (3 + 4) * sizeof(float), reinterpret_cast<void*>(0)
    );
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribDivisor(positionAttribute, 1);

    glVertexAttribPointer(
        orientationAttribute, 4, GL_FLOAT, GL_FALSE,
        (3 + 4) * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float))
    );
    glEnableVertexAttribArray(orientationAttribute);
    glVertexAttribDivisor(orientationAttribute, 1);

    triangleCount = static_cast<GLuint>(mesh.count);
    particleCount = 0;
}

void ParticleSystem::draw(Shader &shader) {
    if (particleCount > 0) {
        glBindVertexArray(mesh.vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, triangleCount, particleCount);
    }
}

void ParticleSystem::add(glm::vec3 position) {
    if (particleCount < capacity) {
        glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
        glBufferSubData(
            GL_ARRAY_BUFFER, particleCount * (3 + 4) * sizeof(float),
            3 * sizeof(float), &position
        );
        particleCount++;
    }
}

