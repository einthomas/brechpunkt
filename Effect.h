#pragma once

#include <initializer_list>

#include <GL/glew.h>
#include "Shader.h"

struct EffectInput {
    const char* identifier;
    GLuint textureUnit;
};

struct EffectOutput {
    const char* identifier;
    GLuint textureUnit;
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
    ~Effect();

    void render();

private:
    GLsizei textureCount;
    GLuint textures[8];
};
