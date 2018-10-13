#include "Effect.h"


Effect::Effect(
    const char *fragmentShaderPath, int width, int height,
    std::initializer_list<EffectInput> inputs,
    std::initializer_list<EffectOutput> outputs
) : shader("shaders/effect.vert", fragmentShaderPath) {

    inputCount = static_cast<int>(inputs.size());
    outputCount = static_cast<int>(outputs.size());

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint drawBuffers[8]{GL_NONE};

    glGenTextures(static_cast<GLsizei>(outputs.size()), outputTextures);
    glActiveTexture(GL_TEXTURE0);

    for (unsigned int i = 0; i < outputs.size(); i++) {
        auto output = *(outputs.begin() + i);

        output.textureName = outputTextures[i];

        GLint location = glGetFragDataLocation(
            shader.program, output.identifier
        );

        if (location != -1) {
            // output is used in shader
            drawBuffers[location] = GL_COLOR_ATTACHMENT0 + i;

            glBindTexture(GL_TEXTURE_2D, outputTextures[i]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexStorage2D(
                GL_TEXTURE_2D, 1, output.internalFormat, width, height
            );

            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                GL_TEXTURE_2D, outputTextures[i], 0
            );
        }
    }

    glDrawBuffers(8, drawBuffers);

    shader.use();

    for (unsigned int i = 0; i < inputs.size(); i++) {
        auto input = *(inputs.begin() + i);

        inputTextures[i] = input.textureName;
        inputTextureTargets[i] = input.textureTarget;

        glUniform1i(
            glGetUniformLocation(shader.program, input.identifier), i
        );
    }
}

Effect::~Effect() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(outputCount, outputTextures);
}

void Effect::render() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    for (int i = 0; i < inputCount; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(inputTextureTargets[i], inputTextures[i]);
    }
    shader.use();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
