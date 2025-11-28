#include <cc/gfx/framebuffer/framebuffer.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/gfx/texture/texture.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>

namespace cc::gfx {

//NOTE: Forward declaration for OpenGL factory
scope<Framebuffer> CreateOpenGLFramebuffer(
    Device* device,
    u32 width,
    u32 height,
    const std::unordered_map<u32, Texture*>& colorAttachments,
    Texture* depthAttachment
);

Framebuffer::Builder Framebuffer::Create(Device* device, u32 width, u32 height) {
    Builder builder;
    builder.device_ = device;
    builder. width_ = width;
    builder.height_ = height;
    return builder;
}

Framebuffer::Builder& Framebuffer::Builder::AttachColor(u32 index, Texture* texture) {
    if (texture == nullptr) {
        log::Warn("Attempting to attach null color texture at index {}", index);
        return *this;
    }
    colorAttachments_[index] = texture;
    return *this;
}

Framebuffer::Builder& Framebuffer::Builder::AttachDepth(Texture* texture) {
    if (texture == nullptr) {
        log::Warn("Attempting to attach null depth texture");
        return *this;
    }
    depthAttachment_ = texture;
    return *this;
}

Framebuffer::Builder& Framebuffer::Builder::AttachDepthStencil(Texture* texture) {
    return AttachDepth(texture);
}

scope<Framebuffer> Framebuffer::Builder::Build() {
    if (device_ == nullptr) {
        log::Critical("Device is required to create Framebuffer");
        throw std::runtime_error("Device is null");
    }

    if (width_ == 0 || height_ == 0) {
        log::Critical("Framebuffer dimensions must be non-zero");
        throw std::runtime_error("Invalid framebuffer dimensions");
    }

    if (colorAttachments_.empty() && depthAttachment_ == nullptr) {
        log::Critical("Framebuffer must have at least one attachment");
        throw std::runtime_error("No attachments specified");
    }

    switch (device_->GetBackend()) {
        case Backend::OpenGL:
            return CreateOpenGLFramebuffer(device_, width_, height_, colorAttachments_, depthAttachment_);
        default:
            log::Critical("Unsupported backend for Framebuffer creation");
            throw std::runtime_error("Unsupported backend");
    }
}

} // namespace cc::gfx
