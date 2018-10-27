#pragma once

#include <initializer_list>

#include <GL/glew.h>

struct FramebufferAttachementParameters {
    GLenum attachmentPoint;
    GLuint& name;
    GLint internalFormat;
};

GLuint generateFramebuffer(
    int width, int height,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
);

GLuint generateFramebuffer(
    int width, int height, GLenum target,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
);

GLuint generateFramebufferMultisample(
    int width, int height, int samples,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
);
