#include "Framebuffer.h"

GLuint generateFramebuffer(
    int width, int height, bool multisample, int samples,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
) {
    GLuint name;
    glGenFramebuffers(1, &name);
    glBindFramebuffer(GL_FRAMEBUFFER, name);

    GLenum target = multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    GLuint drawBuffers[8];

    for (unsigned int i = 0; i < textures.size(); i++) {
        auto& attachment = *(textures.begin() + i);

        drawBuffers[i] = attachment.attachmentPoint;

        glGenTextures(1, &attachment.name);
        glBindTexture(target, attachment.name);

        if (!multisample) {
            // setting sampler parameters is illegal with multisample
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        if (multisample) {
            glTexStorage2DMultisample(
                target, samples, attachment.internalFormat,
                width, height, true
            );
        } else {
            glTexStorage2D(
                target, 1, attachment.internalFormat, width, height
            );
        }

        glFramebufferTexture2D(
            GL_FRAMEBUFFER, attachment.attachmentPoint,
            target, attachment.name, 0
        );
    }

    glDrawBuffers(textures.size(), drawBuffers);

    for (auto& attachment : renderbuffers) {
        glGenRenderbuffers(1, &attachment.name);
        glBindRenderbuffer(GL_RENDERBUFFER, attachment.name);

        if (multisample) {
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER, samples, attachment.internalFormat,
                width, height
            );
        } else {
            glRenderbufferStorage(
                GL_RENDERBUFFER, attachment.internalFormat, width, height
            );
        }

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, attachment.attachmentPoint,
            GL_RENDERBUFFER, attachment.name
        );
    }

    return name;
}

GLuint generateFramebuffer(
    int width, int height,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
) {
    return generateFramebuffer(
        width, height, false, 0, textures, renderbuffers
    );
}

GLuint generateFramebufferMultisample(
    int width, int height, int samples,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
) {
    return generateFramebuffer(
        width, height, true, samples, textures, renderbuffers
    );
}
