#include <cc/gfx/swapchain/swapchain.hpp>
#include <cc/gfx/window/window.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/gfx/framebuffer/framebuffer.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_framebuffer.hpp"
#include <glad/glad.h>
#include <stdexcept>

namespace cc::gfx {

Swapchain::~Swapchain() {
    framebuffer_.reset();
    log::Info("Swapchain destroyed");
}

[[nodiscard]] scope<Swapchain> Swapchain::Create(Window* window, Device* device) {
    if (window == nullptr) {
        throw std::runtime_error("Window is null");
    }

    if (device == nullptr) {
        throw std::runtime_error("Device is null");
    }

    auto swapchain = scope<Swapchain>(new Swapchain());
    swapchain->Initialize(window, device);
    return swapchain;
}

void Swapchain::Initialize(Window* window, Device* device) {
    window_ = window;
    device_ = device;

    //NOTE: Default framebuffer per backend
    switch (device_->GetBackend()) {
        case Backend::OpenGL:
            framebuffer_ = CreateOpenGLDefaultFramebuffer(
                device_,
                window_->GetWidth(),
                window_->GetHeight()
            );
            break;
        case Backend::Vulkan:
            throw std::runtime_error("Vulkan swapchain backend not implemented");
        case Backend::Metal:
            throw std::runtime_error("Metal swapchain backend not implemented");
    }

    //NOTE: Setup default OpenGL state (OpenGL-specific)
    if (device_->GetBackend() == Backend::OpenGL) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    window_->SetResizeCallback([this](u32 width, u32 height) {
        switch (device_->GetBackend()) {
            case Backend::OpenGL:
                framebuffer_ = CreateOpenGLDefaultFramebuffer(device_, width, height);
                log::Info("Swapchain resized ({}x{})", width, height);
                break;
            case Backend::Vulkan:
                log::Critical("Vulkan swapchain resize not implemented");
                break;
            case Backend::Metal:
                log::Critical("Metal swapchain resize not implemented");
                break;
        }
    });

    initialized_ = true;

    log::Info("Swapchain created ({}x{})", window_->GetWidth(), window_->GetHeight());
}

void Swapchain::Clear(const ClearValue& clearValue) {
    if (device_ == nullptr) {
        return;
    }

    if (device_->GetBackend() == Backend::OpenGL) {
        glClearColor(clearValue.color.r, clearValue.color.g, clearValue.color.b, clearValue.color.a);
        glClearDepth(static_cast<double>(clearValue.depthStencil.depth));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    log::Critical("Swapchain::Clear not implemented for current backend");
}

void Swapchain::Present() {
    if (window_ != nullptr) {
        window_->SwapBuffers();
    }
}

u32 Swapchain::GetWidth() const {
    return window_ != nullptr ? window_->GetWidth() : 0;
}

u32 Swapchain::GetHeight() const {
    return window_ != nullptr ? window_->GetHeight() : 0;
}

} // namespace cc::gfx
