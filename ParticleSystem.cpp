#include "ParticleSystem.h"

#include "Mesh.h"

ParticleSystem::ParticleSystem(
    int capacity, GLint positionAttribute, GLint orientationAttribute
) {
    MeshInfo mesh("scenes/scene3/", "MusicCube.obj");

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
        (3 + 4) * sizeof(float), reinterpret_cast<void*>(3 + sizeof(float))
    );
    glEnableVertexAttribArray(orientationAttribute);
    glVertexAttribDivisor(orientationAttribute, 1);

    objectVbo = mesh.VBO;
    vao = mesh.VAO;
    triangleCount = mesh.numTriangles;
    particleCount = 1;
}

void ParticleSystem::draw() {
    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, triangleCount, particleCount);
}

