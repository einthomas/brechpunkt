#include "Shader.h"

#include <iostream>
#include <streambuf>
#include <sstream>
#include <map>
#include <utility>
#include <vector>

using namespace std::literals::string_literals;

struct Shader {
    Shader(GLenum type, const std::string& path) : name(0) {
        if (!path.empty()) {
            std::ifstream file(path.c_str());

            if (!file.is_open()) {
                throw std::runtime_error("Couldn't open "s + path);
            }

            std::string source(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>()
            );
            file.close();

            char const* source_c_str = source.c_str();
            auto length = static_cast<GLint>(source.length());

            name = glCreateShader(type);
            glShaderSource(name, 1, &source_c_str, &length);
            glCompileShader(name);

            GLint success;
            GLchar info_log[1024];
            glGetShaderiv(name, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(name, 1024, nullptr, info_log);
                glDeleteShader(name);
                throw std::runtime_error(
                    "Compilation of "s + path + " failed:\n" + info_log
                );
            }
        }
    }

    Shader(Shader&& other) {
        if (name != 0) {
            glDeleteShader(name);
        }
        name = other.name;
        other.name = 0;
    }

    ~Shader() {
        if (name != 0) {
            glDeleteShader(name);
        }
    }

    GLuint name;
};

Program::Program(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) :
	vertexShaderPath(vertexShaderPath), fragmentShaderPath(fragmentShaderPath)
{
    compileProgram(vertexShaderPath, {}, fragmentShaderPath, {});
    storeUniformLocations();
}


void Program::storeUniformLocations() {
    GLint maxUniformNameLength;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);
    GLint uniformCount;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

    for (GLint i = 0; i < uniformCount; i++) {
        GLint uniformNameLength, uniformSize;
        GLenum uniformType;
        std::vector<GLchar>uniformName(maxUniformNameLength, 0);
        glGetActiveUniform(program, i, maxUniformNameLength, &uniformNameLength, &uniformSize, &uniformType, uniformName.data());
        uniformLocations[uniformName.data()] = glGetUniformLocation(program, uniformName.data());
        std::cout << uniformName.data() << std::endl;
    }
}

Program::Program(
    const std::string& vertexShaderPath, const std::string& geometryShaderPath,
    const std::string& fragmentShaderPath
) :
    vertexShaderPath(vertexShaderPath), geometryShaderPath(geometryShaderPath),
    fragmentShaderPath(fragmentShaderPath) {
    compileProgram(vertexShaderPath, geometryShaderPath, fragmentShaderPath, {});
    storeUniformLocations();
}

Program::Program(
    const std::string& computeShaderPath
) :
    computeShaderPath(computeShaderPath)
{
    compileProgram({}, {}, {}, computeShaderPath);
    storeUniformLocations();
}

void Program::compileProgram(
    std::string vertexShaderPath, std::string geometryShaderPath,
    std::string fragmentShaderPath, std::string computeShaderPath
) {
    program = glCreateProgram();

    Shader shaders[] = {
        {GL_VERTEX_SHADER, std::move(vertexShaderPath)},
        {GL_GEOMETRY_SHADER, std::move(geometryShaderPath)},
        {GL_FRAGMENT_SHADER, std::move(fragmentShaderPath)},
        {GL_COMPUTE_SHADER, std::move(computeShaderPath)}
    };

    for (auto& s : shaders) {
        if (s.name != 0) {
            glAttachShader(program, s.name);
        }
    }

    glLinkProgram(program);

    for (auto& s : shaders) {
        if (s.name != 0) {
            glDetachShader(program, s.name);
        }
    }

	GLint success;
    GLchar *infoLog;
    GLint infoLogLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
	infoLog = new GLchar[infoLogLength];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
        glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog);
		std::cout << "ERROR::SHADERPROGRAM::LINKING\n" << infoLog << std::endl;
	}
    delete[] infoLog;
}

void Program::use() {
	glUseProgram(program);
}

void Program::reload() {
	glDeleteProgram(program);
    compileProgram(vertexShaderPath, geometryShaderPath, fragmentShaderPath, {});
}

void Program::setFloat(const std::string& name, GLfloat value) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glUniform1f(location->second, value);
    }
}

void Program::setInteger(const std::string& name, GLint value) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glUniform1i(location->second, value);
    }
}

void Program::setVector2f(const std::string& name, GLfloat x, GLfloat y) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glUniform2f(location->second, x, y);
    }
}

void Program::setVector2f(const std::string& name, glm::vec2 value) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glUniform2f(location->second, value.x, value.y);
    }
}

void Program::setVector3f(const std::string& name, GLfloat x, GLfloat y, GLfloat z) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glUniform3f(location->second, x, y, z);
    }
}

void Program::setVector3f(const std::string& name, glm::vec3 value) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glUniform3f(location->second, value.x, value.y, value.z);
    }
}

void Program::setMatrix4(const std::string& name, const glm::mat4 &value) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glUniformMatrix4fv(location->second, 1, GL_FALSE, glm::value_ptr(value));
    }
}

void Program::setTexture2D(
    const std::string& name, GLenum activeTexture, GLuint texture, GLuint slot
) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glActiveTexture(activeTexture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(location->second, slot);
    }
}

void Program::setTextureCubeMap(
    const std::string& name, GLenum activeTexture, GLuint texture, GLuint slot
) {
    auto location = uniformLocations.find(name);
    if (location != uniformLocations.end()) {
        glActiveTexture(activeTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glUniform1i(location->second, slot);
    }
}
