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
        const std::unordered_map<u32, Texture*>& colorAttachments,
        Texture* depthAttachment
    );
    ~OpenGLFramebufferImpl() override;

    void Bind() const override;
    void Unbind() const override;
    [[nodiscard]] u32 GetHandle() const noexcept override { return handle_; }

    [[nodiscard]] bool CheckComplete() const;

private:
    void AttachTexture(unsigned int attachmentPoint, Texture* texture) const;

    u32 handle_{0};
    u32 width_{0};
    u32 height_{0};
};

class OpenGLFramebuffer final : public Framebuffer {
public:
    OpenGLFramebuffer(
        u32 width,
        u32 height,
        scope<FramebufferImpl> impl,
        std::unordered_map<u32, Texture*> colorTextures,
        Texture* depthTexture
    );
    ~OpenGLFramebuffer() override;

    [[nodiscard]] u32 GetWidth() const noexcept override { return width_; }
    [[nodiscard]] u32 GetHeight() const noexcept override { return height_; }
    [[nodiscard]] u32 GetHandle() const noexcept override { return impl_->GetHandle(); }

    void Bind() const override;
    void Unbind() const override;

    [[nodiscard]] Texture* GetColorTexture(u32 index) const override;
    [[nodiscard]] Texture* GetDepthTexture() const override;

private:
    u32 width_{0};
    u32 height_{0};
    scope<FramebufferImpl> impl_;
    std::unordered_map<u32, Texture*> colorTextures_;
    Texture* depthTexture_{nullptr};
};

[[nodiscard]] scope<Framebuffer> CreateOpenGLFramebuffer(
    Device* device,
    u32 width,
    u32 height,
    const std::unordered_map<u32, Texture*>& colorAttachments,
    Texture* depthAttachment
);

[[nodiscard]] scope<Framebuffer> CreateOpenGLDefaultFramebuffer(
    Device* device,
    u32 width,
    u32 height
);

} // namespace cc::gfx
