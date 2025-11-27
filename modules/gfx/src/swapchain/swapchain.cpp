#include <cc/gfx/swapchain/swapchain.hpp>
#include <cc/gfx/window/window.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/gfx/framebuffer/framebuffer.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>
#include <stdexcept>

namespace cc::gfx {

Swapchain::~Swapchain() {
    framebuffer_.reset();
    log::Info("Swapchain destroyed");
}

scope<Swapchain> Swapchain::Create(Window* window, Device* device) {
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

    framebuffer_ = device_->CreateDefaultFramebuffer(window_->GetWidth(), window_->GetHeight());

    //NOTE: Setup default OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //NOTE: Setup window resize callback to update framebuffer
    window_->SetResizeCallback([this](u32 width, u32 height) {
        framebuffer_ = device_->CreateDefaultFramebuffer(width, height);
        log::Info("Swapchain resized ({}x{})", width, height);
    });

    initialized_ = true;

    log::Info("Swapchain created ({}x{})", window_->GetWidth(), window_->GetHeight());
}

void Swapchain::Clear(const ClearValue& clearValue) {
    glClearColor(clearValue.color.r, clearValue.color.g, clearValue.color.b, clearValue.color.a);
    glClearDepth(static_cast<double>(clearValue.depthStencil.depth));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Swapchain::Present() {
    window_->SwapBuffers();
}

u32 Swapchain::GetWidth() const {
    return window_->GetWidth();
}

u32 Swapchain::GetHeight() const {
    return window_->GetHeight();
}

} // namespace cc::gfx
