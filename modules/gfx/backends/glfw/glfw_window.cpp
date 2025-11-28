#include "glfw_window.hpp"
#include <cc/core/logger.hpp>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace cc::gfx {

constexpr int OPENGL_MAJOR_VERSION = 4;
constexpr int OPENGL_MINOR_VERSION = 6;

void GLFWWindowImpl::ErrorCallback(int error, const char* description) {
    log::Error("GLFW Error ({}): {}", error, description);
}

void GLFWWindowImpl::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (self != nullptr) {
        self->width_ = static_cast<u32>(width);
        self->height_ = static_cast<u32>(height);

        if (self->resizeCallback_) {
            self->resizeCallback_(self->width_, self->height_);
        }
    }
}

void GLFWWindowImpl::WindowCloseCallback(GLFWwindow* window) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (self != nullptr) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void GLFWWindowImpl::InitGLFW() {
    if (s_initialized) {
        return;
    }

    glfwSetErrorCallback(ErrorCallback);

    if (glfwInit() == 0) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    s_initialized = true;
    log::Info("GLFW initialized");
}

void GLFWWindowImpl::TerminateGLFW() {
    if (!s_initialized) {
        return;
    }

    glfwTerminate();
    s_initialized = false;
    log::Info("GLFW terminated");
}

GLFWWindowImpl::~GLFWWindowImpl() {
    if (handle_ != nullptr) {
        glfwDestroyWindow(handle_);
        handle_ = nullptr;
        log::Info("Window destroyed: '{}'", title_);
    }

    if (s_windowCount > 0) {
        --s_windowCount;
        if (s_windowCount == 0) {
            TerminateGLFW();
        }
    }
}

scope<GLFWWindowImpl> GLFWWindowImpl::Create(const WindowConfig& config) {
    InitGLFW();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, config.decorated ? GLFW_TRUE : GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWmonitor* monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    GLFWwindow* glfwWindow = glfwCreateWindow(
        static_cast<int>(config.width),
        static_cast<int>(config.height),
        config.title.c_str(),
        monitor,
        nullptr
    );

    if (glfwWindow == nullptr) {
        if (s_windowCount == 0) {
            TerminateGLFW();
        }
        throw std::runtime_error("Failed to create GLFW window");
    }

    auto window = scope<GLFWWindowImpl>(new GLFWWindowImpl());
    window->handle_ = glfwWindow;
    window->title_ = config.title;
    window->width_ = config.width;
    window->height_ = config.height;
    window->resizable_ = config.resizable;
    window->vsync_ = config.vsync;
    window->fullscreen_ = config.fullscreen;
    window->decorated_ = config.decorated;

    ++s_windowCount;

    glfwSetWindowUserPointer(glfwWindow, window.get());
    glfwSetFramebufferSizeCallback(glfwWindow, FramebufferSizeCallback);
    glfwSetWindowCloseCallback(glfwWindow, WindowCloseCallback);

    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(config.vsync ? 1 : 0);

    log::Info("Window created: '{}' ({}x{})", config.title, config.width, config.height);

    return window;
}

bool GLFWWindowImpl::ShouldClose() const {
    return glfwWindowShouldClose(handle_) == GLFW_TRUE;
}

void GLFWWindowImpl::PollEvents() {
    glfwPollEvents();
}

void GLFWWindowImpl::SwapBuffers() {
    glfwSwapBuffers(handle_);
}

void GLFWWindowImpl::Close() {
    glfwSetWindowShouldClose(handle_, GLFW_TRUE);
}

u32 GLFWWindowImpl::GetWidth() const {
    return width_;
}

u32 GLFWWindowImpl::GetHeight() const {
    return height_;
}

std::string_view GLFWWindowImpl::GetTitle() const {
    return title_;
}

bool GLFWWindowImpl::IsVSync() const {
    return vsync_;
}

bool GLFWWindowImpl::IsFullscreen() const {
    return fullscreen_;
}

bool GLFWWindowImpl::IsResizable() const {
    return resizable_;
}

void GLFWWindowImpl::SetVSync(bool enabled) {
    vsync_ = enabled;
    glfwMakeContextCurrent(handle_);
    glfwSwapInterval(enabled ? 1 : 0);
}

void GLFWWindowImpl::SetTitle(std::string_view title) {
    title_ = std::string(title);
    glfwSetWindowTitle(handle_, title_.c_str());
}

void GLFWWindowImpl::SetSize(u32 width, u32 height) {
    width_ = width;
    height_ = height;
    glfwSetWindowSize(handle_, static_cast<int>(width), static_cast<int>(height));
}

void* GLFWWindowImpl::GetNativeHandle() const {
    return static_cast<void*>(handle_);
}

void GLFWWindowImpl::SetResizeCallback(WindowResizeCallback callback) {
    resizeCallback_ = std::move(callback);
}

} // namespace cc::gfx
