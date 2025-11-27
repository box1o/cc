#pragma once
#include <cc/gfx/framebuffer/framebuffer.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>
#include <unordered_map>

namespace cc::gfx {

class Device;

class OpenGLFramebufferImpl final : public FramebufferImpl {
public:
    OpenGLFramebufferImpl(
        u32 width,
        u32 height,
        const std::unordered_map<u32, ref<Texture>>& colorAttachments,
        ref<Texture> depthAttachment
    );
    ~OpenGLFramebufferImpl() override;

    void Bind() const override;
    void Unbind() const override;
    u32 GetHandle() const override { return handle_; }

    bool CheckComplete() const;

private:
    void AttachTexture(unsigned int attachmentPoint, ref<Texture> texture) const;

    u32 handle_{0};
    u32 width_;
    u32 height_;
};

class OpenGLFramebuffer final : public Framebuffer {
public:
    OpenGLFramebuffer(
        u32 width,
        u32 height,
        scope<FramebufferImpl> impl,
        std::unordered_map<u32, ref<Texture>> colorTextures,
        ref<Texture> depthTexture
    );
    ~OpenGLFramebuffer() override;

    u32 GetWidth() const override { return width_; }
    u32 GetHeight() const override { return height_; }
    u32 GetHandle() const override { return impl_->GetHandle(); }

    void Bind() const override;
    void Unbind() const override;

    ref<Texture> GetColorTexture(u32 index) const override;
    ref<Texture> GetDepthTexture() const override;

private:
    u32 width_;
    u32 height_;
    scope<FramebufferImpl> impl_;
    std::unordered_map<u32, ref<Texture>> colorTextures_;
    ref<Texture> depthTexture_;
};

scope<Framebuffer> CreateOpenGLFramebuffer(
    Device* device,
    u32 width,
    u32 height,
    const std::unordered_map<u32, ref<Texture>>& colorAttachments,
    ref<Texture> depthAttachment
);

scope<Framebuffer> CreateOpenGLDefaultFramebuffer(Device* device, u32 width, u32 height);

} // namespace cc::gfx
