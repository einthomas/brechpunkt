#pragma once

#include <initializer_list>
#include <vector>

#include <GL/glew.h>
#include "Shader.h"

struct EffectInputParameter {
    const char* identifier;
    GLenum textureTarget;
    GLuint textureName;
};

struct EffectOutputParameter {
    const char* identifier;
    GLuint& textureName;
    GLint internalFormat;
};

struct EffectInput {
    GLuint textureName;
    GLenum textureTarget;
};

class Effect {
public:
    Program shader;
    GLuint framebuffer;

    Effect(
        const char* fragmentShaderPath, int width, int height,
        std::initializer_list<EffectInputParameter> inputs,
        std::initializer_list<EffectOutputParameter> outputs
    );
    Effect(
        const char* fragmentShaderPath, int width, int height,
        std::initializer_list<EffectInputParameter> inputs,
        GLuint framebuffer
    );
    ~Effect();

    void render();

private:
    Effect(
        const char* fragmentShaderPath, int width, int height,
        std::initializer_list<EffectInputParameter> inputs,
        std::initializer_list<EffectOutputParameter> outputs,
        GLuint framebuffer
    );

    GLsizei outputCount;
    GLuint outputTextures[8];

    std::vector<EffectInput> inputs;
};
