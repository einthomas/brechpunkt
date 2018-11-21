#pragma once

#include <initializer_list>

#include <GL/glew.h>
#include "Shader.h"

struct EffectInput {
    const char* identifier;
    GLenum textureTarget;
    GLuint textureName;
};

struct EffectOutput {
    const char* identifier;
    GLuint& textureName;
    GLint internalFormat;
};

class Effect {
public:
    Program shader;
    GLuint framebuffer;

    Effect(
        const char* fragmentShaderPath, int width, int height,
        std::initializer_list<EffectInput> inputs,
        std::initializer_list<EffectOutput> outputs
    );
    Effect(
        const char* fragmentShaderPath, int width, int height,
        std::initializer_list<EffectInput> inputs,
        GLuint framebuffer
    );
    ~Effect();

    void render();

private:
    Effect(
        const char* fragmentShaderPath, int width, int height,
        std::initializer_list<EffectInput> inputs,
        std::initializer_list<EffectOutput> outputs,
        GLuint framebuffer
    );

    GLsizei inputCount, outputCount;
    GLuint outputTextures[8];
    GLuint inputTextures[8];
    GLenum inputTextureTargets[8];
};
