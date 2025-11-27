#include "gl_sampler.hpp"
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>

namespace cc::gfx {

namespace {

unsigned int GetGLFilter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear:  return GL_LINEAR;
    }
    return GL_LINEAR;
}

unsigned int GetGLWrap(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat:        return GL_REPEAT;
        case TextureWrap::MirrorRepeat:  return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:   return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
    }
    return GL_REPEAT;
}

} // anonymous namespace

OpenGLSamplerImpl::OpenGLSamplerImpl(const SamplerConfig& config) {
    glCreateSamplers(1, &handle_);
    if (handle_ == 0) {
        log::Critical("Failed to create OpenGL sampler");
        return;
    }

    glSamplerParameteri(handle_, GL_TEXTURE_MIN_FILTER, static_cast<int>(GetGLFilter(config.minFilter)));
    glSamplerParameteri(handle_, GL_TEXTURE_MAG_FILTER, static_cast<int>(GetGLFilter(config.magFilter)));
    glSamplerParameteri(handle_, GL_TEXTURE_WRAP_S, static_cast<int>(GetGLWrap(config.wrapU)));
    glSamplerParameteri(handle_, GL_TEXTURE_WRAP_T, static_cast<int>(GetGLWrap(config.wrapV)));
    glSamplerParameteri(handle_, GL_TEXTURE_WRAP_R, static_cast<int>(GetGLWrap(config.wrapW)));

    if (config.maxAnisotropy > 1.0f) {
        glSamplerParameterf(handle_, GL_TEXTURE_MAX_ANISOTROPY, config.maxAnisotropy);
    }

    glSamplerParameterfv(handle_, GL_TEXTURE_BORDER_COLOR, config.borderColor);

    log::Trace("OpenGL Sampler created (handle={})", handle_);
}

OpenGLSamplerImpl::~OpenGLSamplerImpl() {
    if (handle_ != 0) {
        glDeleteSamplers(1, &handle_);
        log::Trace("OpenGL Sampler destroyed (handle={})", handle_);
        handle_ = 0;
    }
}

void OpenGLSamplerImpl::Bind(u32 slot) const {
    glBindSampler(slot, handle_);
}

scope<Sampler> CreateOpenGLSampler(Device* /*device*/, const SamplerConfig& config) {
    auto impl = scope<SamplerImpl>(new OpenGLSamplerImpl(config));
    return scope<Sampler>(new Sampler(config, std::move(impl)));
}

} // namespace cc::gfx
