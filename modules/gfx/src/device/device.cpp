#include <cc/gfx/device/device.hpp>
#include <cc/gfx/window/window.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_device.hpp"
#include <stdexcept>

namespace cc::gfx {

[[nodiscard]] Device::Builder Device::Create(Window* window, Backend backend) noexcept {
    Builder builder;
    builder.window_ = window;
    builder.backend_ = backend;
    return builder;
}

Device::Builder& Device::Builder::SetBackend(Backend backend) noexcept {
    backend_ = backend;
    return *this;
}

Device::Builder& Device::Builder::EnableValidation(bool enable) noexcept {
    enableValidation_ = enable;
    return *this;
}

[[nodiscard]] scope<Device> Device::Builder::Build() {
    if (window_ == nullptr) {
        log::Critical("Window is required to create device");
        throw std::runtime_error("Window is null");
    }

    switch (backend_) {
        case Backend::OpenGL:
            return CreateOpenGLDevice(*this);
        case Backend::Vulkan:
            log::Critical("Vulkan backend not implemented");
            throw std::runtime_error("Vulkan backend not implemented");
        case Backend::Metal:
            log::Critical("Metal backend not implemented");
            throw std::runtime_error("Metal backend not implemented");
    }

    log::Critical("Unknown graphics backend");
    throw std::runtime_error("Unknown graphics backend");
}

} // namespace cc::gfx
