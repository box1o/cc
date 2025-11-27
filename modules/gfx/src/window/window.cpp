#include <cc/gfx/window/window.hpp>
#include <cc/core/logger.hpp>
#include "backends/glfw/glfw_window.hpp"
#include <stdexcept>

namespace cc::gfx {

Window::Builder Window::Create() {
    return Builder{};
}

Window::Builder& Window::Builder::SetTitle(std::string_view title) {
    config_.title = title;
    return *this;
}

Window::Builder& Window::Builder::SetSize(u32 width, u32 height) {
    config_. width = width;
    config_. height = height;
    return *this;
}

Window::Builder& Window::Builder::SetResizable(bool resizable) {
    config_.resizable = resizable;
    return *this;
}

Window::Builder& Window::Builder::SetVSync(bool vsync) {
    config_.vsync = vsync;
    return *this;
}

Window::Builder& Window::Builder::SetFullscreen(bool fullscreen) {
    config_.fullscreen = fullscreen;
    return *this;
}

Window::Builder& Window::Builder::SetDecorated(bool decorated) {
    config_.decorated = decorated;
    return *this;
}

Window::Builder& Window::Builder::SetBackend(WindowBackend backend) {
    config_.backend = backend;
    return *this;
}

scope<Window> Window::Builder::Build() {
    switch (config_.backend) {
        case WindowBackend::GLFW:
            return GLFWWindowImpl::Create(config_);
        case WindowBackend::Native:
            log::Critical("Native window backend not implemented");
            throw std::runtime_error("Native window backend not implemented");
    }

    log::Critical("Unknown window backend");
    throw std::runtime_error("Unknown window backend");
}

} // namespace cc::gfx
