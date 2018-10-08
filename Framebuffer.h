#pragma once

#include <initializer_list>

#include <GL/glew.h>

struct FramebufferAttachementParameters {
    GLenum attachmentPoint;
    GLuint& name;
    GLint internalFormat;
    GLenum format;
};

GLuint generateFramebuffer(
    int width, int height,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
);
