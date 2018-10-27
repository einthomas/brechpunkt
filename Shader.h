#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>

class Shader {
public:
	GLuint program = -1;

	Shader() = default;
	Shader(std::string vertexShaderPath, std::string fragmentShaderPath);
    Shader(
        std::string vertexShaderPath, std::string geometryShaderPath,
        std::string fragmentShaderPath
    );
	void use();
	void reload();

	void setFloat(std::string name, GLfloat value);
	void setInteger(std::string name, GLint value);
	void setVector2f(std::string name, GLfloat x, GLfloat y);
	void setVector2f(std::string name, glm::vec2 value);
	void setVector3f(std::string name, GLfloat x, GLfloat y, GLfloat z);
	void setVector3f(std::string name, glm::vec3 value);
	void setMatrix4(std::string name, const glm::mat4 &value) const;
	void setTexture2D(std::string name, GLenum activeTexture, GLuint texture, GLuint loc);
	void setTextureCubeMap(std::string name, GLenum activeTexture, GLuint texture, GLuint loc);

private:
    GLuint loadShader(GLuint shaderType, std::string shaderPath);
    void compileShader(
        std::string vertexShaderPath, std::string geometryShaderPath,
        std::string fragmentShaderPath
    );
	std::string vertexShaderPath;
    std::string geometryShaderPath;
	std::string fragmentShaderPath;
};
