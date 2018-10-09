#pragma once

#include <initializer_list>

#include <GL/glew.h>
#include <Shader.h>

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
    Effect(
        const char* fragmentShaderPath, int width, int height,
        std::initializer_list<EffectInput> inputs,
        std::initializer_list<EffectOutput> outputs
    );
    ~Effect();

    void render();

private:
    Shader shader;
    GLuint framebuffer;
    GLsizei textureCount;
    GLuint textures[8];
};
