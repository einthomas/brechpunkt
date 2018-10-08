#pragma once

#include <GL/glew.h>

#include "Shader.h"

class Mesh {
public:
    GLuint VAO;
    std::string materialName;
    int numTriangles;
    glm::vec3 position;
    
    Mesh(GLuint VAO, std::string materialName, int numTriangles, glm::vec3 position);
    void draw(Shader &shader);
};
