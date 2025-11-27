#include "gl_buffer.hpp"
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>

namespace cc::gfx {

OpenGLBufferImpl::OpenGLBufferImpl(BufferType type, BufferUsage usage, u64 size, const void* data)
    : type_(type), usage_(usage), size_(size) {

    glCreateBuffers(1, &handle_);
    if (handle_ == 0) {
        log::Critical("Failed to create OpenGL buffer");
        return;
    }

    glNamedBufferData(handle_, static_cast<long long>(size_), data, GetGLUsage());

    log::Trace("OpenGL buffer created (handle={}, size={})", handle_, size_);
}

OpenGLBufferImpl::~OpenGLBufferImpl() {
    if (handle_ != 0) {
        glDeleteBuffers(1, &handle_);
        log::Trace("OpenGL buffer destroyed (handle={})", handle_);
        handle_ = 0;
    }
}

void OpenGLBufferImpl::Update(const void* data, u64 size, u64 offset) {
    if (handle_ == 0) return;
    glNamedBufferSubData(handle_, static_cast<long long>(offset), static_cast<long long>(size), data);
}

unsigned int OpenGLBufferImpl::GetGLTarget() const {
    switch (type_) {
        case BufferType::Vertex:  return GL_ARRAY_BUFFER;
        case BufferType::Index:   return GL_ELEMENT_ARRAY_BUFFER;
        case BufferType::Uniform: return GL_UNIFORM_BUFFER;
        case BufferType::Storage: return GL_SHADER_STORAGE_BUFFER;
        case BufferType::Staging: return GL_COPY_WRITE_BUFFER;
    }
    return GL_ARRAY_BUFFER;
}

unsigned int OpenGLBufferImpl::GetGLUsage() const {
    switch (usage_) {
        case BufferUsage::Static:  return GL_STATIC_DRAW;
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
    }
    return GL_STATIC_DRAW;
}

scope<Buffer> CreateOpenGLBuffer(Device* /*device*/, BufferType type, u64 size, BufferUsage usage, const void* data) {
    auto impl = scope<BufferImpl>(new OpenGLBufferImpl(type, usage, size, data));
    return scope<Buffer>(new Buffer(type, usage, size, std::move(impl)));
}

} // namespace cc::gfx
