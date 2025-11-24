
#include "window/window.hpp"
#include "backends/glfw/glfw_window.hpp"
#include "logger.hpp"
#include <stdexcept>

namespace cc::gfx {

namespace{
static WindowBackend backend_ = WindowBackend::GLFW;
}


Window::Builder Window::Create(WindowBackend backend ){
    backend_ = backend;
    return Window::Builder{};
}



ref<Window> Window::Builder::Build(){
    switch (backend_)
    {
    case WindowBackend::GLFW:
        return std::static_pointer_cast<Window>(GLFWWindowImpl::CreateFromBuilder(*this));
        break;
    case WindowBackend::NATIVE:
        log::Critical("Window backend not implemented yet!");
        throw std::runtime_error("Not implemented yet!");  // Note: should be 'throw', not just 'std::runtime_error'
        break;
    }
    return {};
}






} // namespace cc::gfx
