#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <unordered_map>

class Program {
public:
    GLuint program = -1;

    Program() = default;
    Program(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    Program(
        const std::string& vertexShaderPath, const std::string& geometryShaderPath,
        const std::string& fragmentShaderPath
    );
    Program(const std::string& computeShaderPath);

	void use();
	void reload();

	void setFloat(const std::string& name, GLfloat value);
	void setInteger(const std::string& name, GLint value);
	void setVector2f(const std::string& name, GLfloat x, GLfloat y);
	void setVector2f(const std::string& name, glm::vec2 value);
	void setVector3f(const std::string& name, GLfloat x, GLfloat y, GLfloat z);
	void setVector3f(const std::string& name, glm::vec3 value);
	void setMatrix4(const std::string& name, const glm::mat4 &value);
	void setTexture2D(const std::string& name, GLenum activeTexture, GLuint texture, GLuint loc);
	void setTextureCubeMap(const std::string& name, GLenum activeTexture, GLuint texture, GLuint loc);

private:
    void compileProgram(
        std::string vertexShaderPath, std::string geometryShaderPath,
        std::string fragmentShaderPath,
        std::string computeShaderPath
    );
    void storeUniformLocations();

    std::unordered_map<std::string, GLint> uniformLocations;
	std::string vertexShaderPath;
    std::string geometryShaderPath;
	std::string fragmentShaderPath;
    std::string computeShaderPath;
};
