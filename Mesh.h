#pragma once

#include <GL/glew.h>

#include "Shader.h"

GLuint loadTexture(std::string textureFileName);

struct MeshInfo {
    GLuint VAO, VBO;
    std::string materialName;
    int numTriangles;

	MeshInfo() = default;

    MeshInfo(GLuint VAO, GLuint VBO, std::string materialName, int numTriangles) :
        VAO(VAO), VBO(VBO),
		materialName(materialName),
		numTriangles(numTriangles)
	{
	}

    MeshInfo(std::string basedir, std::string objFileName);
};

class Mesh {
public:
	glm::vec3 diffuseColor, emissionColor;
    bool useDiffuseTexture, useReflectionTexture, useNormalTexture;
    GLuint diffuseTexture, reflectionTexture, normalTexture;
    glm::mat4 model;
    GLuint vao;
    GLsizei count;
    
    Mesh() = default;
    Mesh(
        MeshInfo meshInfo, glm::mat4 model, glm::vec3 diffuseColor,
        glm::vec3 emissionColor
    );
    void draw(Program &shader);
    void drawRefractive(Program &shader);
    void setUniforms(Program &shader);
};
