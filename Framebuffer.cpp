#include "Framebuffer.h"

#include <stdexcept>


GLuint generateFramebuffer(
    int width, int height, bool multisample, int samples, GLenum target,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
) {
    GLuint name;
    glGenFramebuffers(1, &name);
    glBindFramebuffer(GL_FRAMEBUFFER, name);

    GLuint drawBuffers[8]{GL_NONE};

    for (unsigned int i = 0; i < textures.size(); i++) {
        auto& attachment = *(textures.begin() + i);

        if (
            attachment.attachmentPoint != GL_DEPTH_ATTACHMENT &&
            attachment.attachmentPoint != GL_STENCIL_ATTACHMENT &&
            attachment.attachmentPoint != GL_DEPTH_STENCIL_ATTACHMENT
        ) {
            drawBuffers[i] = attachment.attachmentPoint;
        }

        glGenTextures(1, &attachment.name);
        glBindTexture(target, attachment.name);

        if (!multisample) {
            // setting sampler parameters is illegal with multisample
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        if (
            attachment.attachmentPoint == GL_DEPTH_ATTACHMENT ||
            attachment.attachmentPoint == GL_DEPTH_STENCIL_ATTACHMENT
        ) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
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

        glFramebufferTexture(
            GL_FRAMEBUFFER, attachment.attachmentPoint, attachment.name, 0
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

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
        throw std::runtime_error("Framebuffer has incomplete attachment");
    } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
        throw std::runtime_error("Framebuffer needs at least one attachment");
    } else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) {
        throw std::runtime_error("Framebuffer has no draw buffers");
    } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
        throw std::runtime_error(
            "Framebuffer attachments must have same number and "
            "location of samples"
        );
    } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
        throw std::runtime_error("Framebuffer internal format not supported");
    } else if (status == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS) {
        throw std::runtime_error(
            "Framebuffer attachments must have same target and number of layers"
        );
    } else if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error(
            "Framebuffer is incomplete for unknown reason"
        );
    }

    return name;
}

GLuint generateFramebuffer(int width, int height,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
) {
    return generateFramebuffer(
        width, height, false, 0, GL_TEXTURE_2D,
        textures, renderbuffers
    );
}

GLuint generateFramebuffer(
    int width, int height, GLenum target,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
) {
    return generateFramebuffer(
        width, height, false, 0, target, textures, renderbuffers
    );
}


GLuint generateFramebufferMultisample(
    int width, int height, int samples,
    std::initializer_list<FramebufferAttachementParameters> textures,
    std::initializer_list<FramebufferAttachementParameters> renderbuffers
) {
    return generateFramebuffer(
        width, height, true, samples, GL_TEXTURE_2D_MULTISAMPLE,
        textures, renderbuffers
    );
}
