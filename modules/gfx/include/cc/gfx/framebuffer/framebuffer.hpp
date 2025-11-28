#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <unordered_map>

namespace cc::gfx {

class Framebuffer {
public:
    class Builder {
    public:
        Builder& AttachColor(u32 index, Texture* texture);
        Builder& AttachDepth(Texture* texture);
        Builder& AttachDepthStencil(Texture* texture);
        [[nodiscard]] scope<Framebuffer> Build();

    private:
        Device* device_{nullptr};
        u32 width_{0};
        u32 height_{0};
        std::unordered_map<u32, Texture*> colorAttachments_;
        Texture* depthAttachment_{nullptr};

        friend class Framebuffer;
    };

    virtual ~Framebuffer() = default;

    [[nodiscard]] static Builder Create(Device* device, u32 width, u32 height) noexcept;

    [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
    [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    [[nodiscard]] virtual Texture* GetColorTexture(u32 index = 0) const = 0;
    [[nodiscard]] virtual Texture* GetDepthTexture() const = 0;

protected:
    Framebuffer() = default;
};

class FramebufferImpl {
public:
    virtual ~FramebufferImpl() = default;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

protected:
    FramebufferImpl() = default;
};

} // namespace cc::gfx
