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
    glm::vec3 scale;
    glm::vec3 rotation;
    glm::vec3 position;

    float emissionColorBrightness;
	glm::vec3 diffuseColor, emissionColor;
    bool useDiffuseTexture, useReflectionTexture, useNormalTexture;
    GLuint diffuseTexture, reflectionTexture, normalTexture;
    //glm::mat4 model;
    GLuint vao;
    GLsizei count;
    float minHeight, maxHeight;
    
    Mesh() = default;
    Mesh(
        MeshInfo meshInfo, glm::vec3 diffuseColor,
        float emissionColorBrightness, glm::vec3 emissionColor,
        glm::vec3 position = glm::vec3(0.0f),
        glm::vec3 scale = glm::vec3(1.0f), glm::vec3 rotation = glm::vec3(0.0f)
    );
    void draw(Program &shader);
    void setUniforms(Program &shader);
    void setModelMatrix(glm::mat4 model);

private:
    bool usePresetModelMatrix = false;
    glm::mat4 model;
};
