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
        GL_ARRAY_BUFFER, capacity * (4 + 4) * sizeof(float),
        nullptr, GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        positionAttribute, 3, GL_FLOAT, GL_FALSE,
        (4 + 4) * sizeof(float), reinterpret_cast<void*>(0)
    );
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribDivisor(positionAttribute, 1);

    glVertexAttribPointer(
        orientationAttribute, 4, GL_FLOAT, GL_FALSE,
        (4 + 4) * sizeof(float), reinterpret_cast<void*>(4 * sizeof(float))
    );
    glEnableVertexAttribArray(orientationAttribute);
    glVertexAttribDivisor(orientationAttribute, 1);

    glGenBuffers(1, &physicVbo);
    glBindBuffer(GL_ARRAY_BUFFER, physicVbo);
    glBufferData(
        GL_ARRAY_BUFFER, capacity * (3 + 3) * sizeof(float),
        nullptr, GL_DYNAMIC_DRAW
    );

    triangleCount = static_cast<GLuint>(mesh.count);
    particleCount = 0;
}

void ParticleSystem::draw(Program &) {
    if (particleCount > 0) {
        glBindVertexArray(mesh.vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, triangleCount, particleCount);
    }
}

void ParticleSystem::add(glm::vec3 position, glm::quat orientation) {
    if (particleCount < capacity) {
        float values[8] = {
            position.x, position.y, position.z, 0,
            orientation.x, orientation.y, orientation.z, orientation.w
        };

        glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
        glBufferSubData(
            GL_ARRAY_BUFFER, particleCount * sizeof(values),
            sizeof(values), values
        );
        particleCount++;
    }
}
