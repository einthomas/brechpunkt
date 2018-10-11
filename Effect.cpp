#include "Effect.h"


Effect::Effect(
    const char *fragmentShaderPath, int width, int height,
    std::initializer_list<EffectInput> inputs,
    std::initializer_list<EffectOutput> outputs
) : shader("shaders/effect.vert", fragmentShaderPath) {

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint drawBuffers[8]{GL_NONE};

    glGenTextures(static_cast<GLsizei>(outputs.size()), textures);

    for (unsigned int i = 0; i < outputs.size(); i++) {
        auto output = *(outputs.begin() + i);

        GLint location = glGetFragDataLocation(
            shader.program, output.identifier
        );

        if (location != -1) {
            // output is used in shader
            drawBuffers[location] = GL_COLOR_ATTACHMENT0 + i;

            glBindTexture(GL_TEXTURE_2D, textures[i]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexStorage2D(
                GL_TEXTURE_2D, 1, output.internalFormat, width, height
            );

            glBindTextureUnit(output.textureUnit, textures[i]);

            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                GL_TEXTURE_2D, textures[i], 0
            );
        }
    }

    glDrawBuffers(8, drawBuffers);

    shader.use();

    for (auto input : inputs) {
        glUniform1i(
            glGetUniformLocation(shader.program, input.identifier),
            input.textureUnit
        );
    }
}

Effect::~Effect() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(textureCount, textures);
}

void Effect::render() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    shader.use();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
