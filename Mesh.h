#pragma once

#include <GL/glew.h>

#include "Shader.h"

static GLuint loadTexture(std::string textureFileName);

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

    MeshInfo(std::string basedir, std::string objFileName);
};

class Mesh {
public:
	glm::vec3 diffuseColor;
    glm::vec3 emissionColor;
    bool useDiffuseTexture, useReflectionTexture, useNormalTexture;
    GLuint diffuseTexture, reflectionTexture, normalTexture;
    glm::mat4 model;
    GLuint vao;
    GLsizei count;
    
    Mesh();
    Mesh(MeshInfo meshInfo, glm::mat4 model, glm::vec3 diffuseColor, glm::vec3 emissionColor);
    void draw(Shader &shader);
};
