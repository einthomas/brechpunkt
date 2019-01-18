#pragma once

#include <GL/glew.h>

#include "Shader.h"

GLuint loadTexture(const std::string& textureFileName);

struct MeshInfo {
    GLuint VAO{}, VBO{};
    std::string materialName;
    int numTriangles;

	MeshInfo() = default;
    MeshInfo(const std::string& basedir, const std::string& fileName);
};

class Mesh {
public:
    float emissionColorBrightness;
	glm::vec3 diffuseColor, emissionColor;
    bool useDiffuseTexture, useReflectionTexture, useNormalTexture;
    GLuint diffuseTexture, reflectionTexture, normalTexture;
    glm::mat4 model;
    GLuint vao;
    GLsizei count;
    float minHeight, maxHeight, currentHeight;
    
    Mesh() = default;
    Mesh(
        MeshInfo meshInfo, glm::mat4 model, glm::vec3 diffuseColor,
        float emissionColorBrightness, glm::vec3 emissionColor
    );
    void draw(Program &shader);
    void setUniforms(Program &shader);
    void resetModelMatrix();

private:
    glm::mat4 modelUnchanged;
};
