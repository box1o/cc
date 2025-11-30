#include "glfw_window.hpp"
#include <cc/core/logger.hpp>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace cc::gfx {

using namespace cc::gfx::events;

constexpr int OPENGL_MAJOR_VERSION = 4;
constexpr int OPENGL_MINOR_VERSION = 6;

static KeyCode MapGlfwKey(int key) {
    return static_cast<KeyCode>(key);
}

static MouseButton MapGlfwButton(int button) {
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
        default:                       return static_cast<MouseButton>(button);
    }
}

void GLFWWindowImpl::ErrorCallback(int error, const char* description) {
    log::Error("GLFW Error ({}): {}", error, description);
}

void GLFWWindowImpl::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self) return;

    self->width_  = static_cast<u32>(width);
    self->height_ = static_cast<u32>(height);

    if (self->resizeCallback_) {
        self->resizeCallback_(self->width_, self->height_);
    }

    if (self->eventBus_) {
        WindowResizeEvent e(self->width_, self->height_);
        self->eventBus_->Emit(e);

        ViewportResizeEvent ve(self->width_, self->height_);
        self->eventBus_->Emit(ve);
    }
}

void GLFWWindowImpl::WindowCloseCallback(GLFWwindow* window) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self) return;

    if (self->eventBus_) {
        WindowCloseEvent e;
        self->eventBus_->Emit(e);
    }

    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GLFWWindowImpl::WindowFocusCallback(GLFWwindow* window, int focused) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    if (focused == GLFW_TRUE) {
        WindowFocusEvent e;
        self->eventBus_->Emit(e);
    } else {
        WindowLostFocusEvent e;
        self->eventBus_->Emit(e);
    }
}

void GLFWWindowImpl::WindowPosCallback(GLFWwindow* window, int xpos, int ypos) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    WindowMovedEvent e(xpos, ypos);
    self->eventBus_->Emit(e);
}

void GLFWWindowImpl::WindowIconifyCallback(GLFWwindow* window, int iconified) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    if (iconified) {
        WindowMinimizedEvent e;
        self->eventBus_->Emit(e);
    } else {
        WindowRestoredEvent e;
        self->eventBus_->Emit(e);
    }
}

void GLFWWindowImpl::WindowMaximizeCallback(GLFWwindow* window, int maximized) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    if (maximized) {
        WindowMaximizedEvent e;
        self->eventBus_->Emit(e);
    } else {
        WindowRestoredEvent e;
        self->eventBus_->Emit(e);
    }
}

void GLFWWindowImpl::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    KeyCode code = MapGlfwKey(key);

    if (action == GLFW_PRESS) {
        KeyPressedEvent e(code, 0);
        self->eventBus_->Emit(e);
    } else if (action == GLFW_REPEAT) {
        KeyPressedEvent e(code, 1);
        self->eventBus_->Emit(e);
    } else if (action == GLFW_RELEASE) {
        KeyReleasedEvent e(code);
        self->eventBus_->Emit(e);
    }
}

void GLFWWindowImpl::CharCallback(GLFWwindow* window, unsigned int codepoint) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    KeyTypedEvent e(codepoint);
    self->eventBus_->Emit(e);
}

void GLFWWindowImpl::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    float fx = static_cast<float>(xpos);
    float fy = static_cast<float>(ypos);
    float dx = 0.0f;
    float dy = 0.0f;

    if (self->hasLastCursor_) {
        dx = fx - static_cast<float>(self->lastCursorX_);
        dy = fy - static_cast<float>(self->lastCursorY_);
    }

    self->lastCursorX_ = xpos;
    self->lastCursorY_ = ypos;
    self->hasLastCursor_ = true;

    MouseMovedEvent e(fx, fy, dx, dy);
    self->eventBus_->Emit(e);
}

void GLFWWindowImpl::CursorEnterCallback(GLFWwindow* window, int entered) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    if (entered == GLFW_TRUE) {
        MouseEnteredEvent e;
        self->eventBus_->Emit(e);
    } else {
        MouseLeftEvent e;
        self->eventBus_->Emit(e);
    }
}

void GLFWWindowImpl::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    float fx = static_cast<float>(xpos);
    float fy = static_cast<float>(ypos);

    MouseButton mb = MapGlfwButton(button);

    if (action == GLFW_PRESS) {
        MouseButtonPressedEvent e(mb, fx, fy);
        self->eventBus_->Emit(e);
    } else if (action == GLFW_RELEASE) {
        MouseButtonReleasedEvent e(mb, fx, fy);
        self->eventBus_->Emit(e);
    }
}

void GLFWWindowImpl::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(window));
    if (!self || !self->eventBus_) return;

    MouseScrolledEvent e(static_cast<float>(xoffset), static_cast<float>(yoffset));
    self->eventBus_->Emit(e);
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
    window->handle_     = glfwWindow;
    window->title_      = config.title;
    window->width_      = config.width;
    window->height_     = config.height;
    window->resizable_  = config.resizable;
    window->vsync_      = config.vsync;
    window->fullscreen_ = config.fullscreen;
    window->decorated_  = config.decorated;

    ++s_windowCount;

    glfwSetWindowUserPointer(glfwWindow, window.get());
    glfwSetFramebufferSizeCallback(glfwWindow, FramebufferSizeCallback);
    glfwSetWindowCloseCallback(glfwWindow, WindowCloseCallback);
    glfwSetWindowFocusCallback(glfwWindow, WindowFocusCallback);
    glfwSetWindowPosCallback(glfwWindow, WindowPosCallback);
    glfwSetWindowIconifyCallback(glfwWindow, WindowIconifyCallback);
    glfwSetWindowMaximizeCallback(glfwWindow, WindowMaximizeCallback);

    glfwSetKeyCallback(glfwWindow, KeyCallback);
    glfwSetCharCallback(glfwWindow, CharCallback);
    glfwSetCursorPosCallback(glfwWindow, CursorPosCallback);
    glfwSetCursorEnterCallback(glfwWindow, CursorEnterCallback);
    glfwSetMouseButtonCallback(glfwWindow, MouseButtonCallback);
    glfwSetScrollCallback(glfwWindow, ScrollCallback);

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
    width_  = width;
    height_ = height;
    glfwSetWindowSize(handle_, static_cast<int>(width), static_cast<int>(height));
}

void* GLFWWindowImpl::GetNativeHandle() const {
    return static_cast<void*>(handle_);
}

void GLFWWindowImpl::SetResizeCallback(WindowResizeCallback callback) {
    resizeCallback_ = std::move(callback);
}

void GLFWWindowImpl::SetEventBus(const ref<events::EventBus>& bus) {
    eventBus_ = bus;
}

} // namespace cc::gfx
