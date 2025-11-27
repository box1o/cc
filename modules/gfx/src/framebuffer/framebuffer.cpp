#include <cc/gfx/framebuffer/framebuffer.hpp>
#include <cc/gfx/texture/texture.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_framebuffer.hpp"
#include <stdexcept>

namespace cc::gfx {

Framebuffer::Builder Framebuffer::Create(Device* device, u32 width, u32 height) {
    Builder builder;
    builder.device_ = device;
    builder.width_ = width;
    builder.height_ = height;
    return builder;
}

Framebuffer::Builder& Framebuffer::Builder::AttachColor(u32 index, ref<Texture> texture) {
    colorAttachments_[index] = texture;
    return *this;
}

Framebuffer::Builder& Framebuffer::Builder::AttachDepth(ref<Texture> texture) {
    depthAttachment_ = texture;
    return *this;
}

Framebuffer::Builder& Framebuffer::Builder::AttachDepthStencil(ref<Texture> texture) {
    depthAttachment_ = texture;
    return *this;
}

scope<Framebuffer> Framebuffer::Builder::Build() {
    if (device_ == nullptr) {
        log::Critical("Device is required to create Framebuffer");
        throw std::runtime_error("Device is null");
    }

    if (width_ == 0 || height_ == 0) {
        log::Critical("Framebuffer size must be greater than 0");
        throw std::runtime_error("Invalid framebuffer size");
    }

    return CreateOpenGLFramebuffer(device_, width_, height_, colorAttachments_, depthAttachment_);
}

} // namespace cc::gfx
