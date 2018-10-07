#pragma once

#include <GL\glew.h>

#include "Shader.h"

class Mesh {
public:
    GLuint VAO;
    std::string materialName;
    int numTriangles;
    
    Mesh(GLuint VAO, std::string materialName, int numTriangles);
    void draw(Shader *shader, glm::mat4 &modelMatrix);
    void update(glm::mat4 &modelMatrix);
};
