#pragma once

#include <GL/glew.h>

#include "Shader.h"

struct MeshInfo {
    GLuint VAO;
    std::string materialName;
    int numTriangles;

	MeshInfo() = default;

	MeshInfo(GLuint VAO, std::string materialName, int numTriangles) :
		VAO(VAO),
		materialName(materialName),
		numTriangles(numTriangles)
	{
	}
};

class Mesh {
public:
    glm::vec3 position;
	glm::vec3 color;
    
    Mesh(MeshInfo meshInfo, glm::vec3 position, glm::vec3 color);
    void draw(Shader &shader);

private:
	MeshInfo meshInfo;
};
