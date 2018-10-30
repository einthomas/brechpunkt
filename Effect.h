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
    Shader shader;
    GLuint framebuffer;

    Effect(
        const char* fragmentShaderPath, int width, int height,
        std::initializer_list<EffectInput> inputs,
        std::initializer_list<EffectOutput> outputs
    );
    Effect(const char* fragmentShaderPath, int width, int height,
        bool multisample, int samples,
        std::initializer_list<EffectInput> inputs,
        std::initializer_list<EffectOutput> outputs
    );
    ~Effect();

    void render();

private:
    GLsizei inputCount, outputCount;
    GLuint outputTextures[8];
    GLuint inputTextures[8];
    GLenum inputTextureTargets[8];
};
