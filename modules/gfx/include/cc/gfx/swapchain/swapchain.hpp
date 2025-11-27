#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

class Swapchain {
public:
    ~Swapchain();

    static scope<Swapchain> Create(Window* window, Device* device);

    void Clear(const ClearValue& clearValue);
    void Present();

    u32 GetWidth() const;
    u32 GetHeight() const;
    Framebuffer* GetFramebuffer() const { return framebuffer_. get(); }

private:
    Swapchain() = default;
    void Initialize(Window* window, Device* device);

    Window* window_{nullptr};
    Device* device_{nullptr};
    scope<Framebuffer> framebuffer_;
    bool initialized_{false};
};

} // namespace cc::gfx
