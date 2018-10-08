#include "Framebuffer.h"

GLuint generateFramebuffer(
    int width, int height,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
) {
    GLuint name;
    glGenFramebuffers(1, &name);
    glBindFramebuffer(GL_FRAMEBUFFER, name);

    for (auto& attachment : textures) {
        glGenTextures(1, &attachment.name);
        glBindTexture(GL_TEXTURE_2D, attachment.name);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(
            GL_TEXTURE_2D, 0, attachment.internalFormat,
            width, height, 0, attachment.format, GL_BYTE, nullptr
        );

        glFramebufferTexture2D(
            GL_FRAMEBUFFER, attachment.attachmentPoint,
            GL_TEXTURE_2D, attachment.name, 0
        );
    }

    for (auto& attachment : renderbuffers) {
        glGenRenderbuffers(1, &attachment.name);
        glBindRenderbuffer(GL_RENDERBUFFER, attachment.name);
        glRenderbufferStorage(
            GL_RENDERBUFFER, attachment.internalFormat,
            width, height
        );

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, attachment.attachmentPoint,
            GL_RENDERBUFFER, attachment.name
        );
    }

    return name;
}
