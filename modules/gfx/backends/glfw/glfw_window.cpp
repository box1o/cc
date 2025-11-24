#include "glfw_window.hpp"


namespace cc::gfx {

static void GLFWErrorCallback(int error, const char* description) {
    log::Error("GLFW Error ({}): {}", error, description);
}

GLFWWindowImpl::~GLFWWindowImpl() {
    if (window_) {
        glfwDestroyWindow(window_);
        log::Info("Window destroyed: {}", title_);
        window_ = nullptr;
    }

    glfwWindowCount_--;
    if (glfwWindowCount_ == 0) {
        TerminateGLFW();
    }
}

void GLFWWindowImpl::InitializeGLFW() {
    if (glfwInitialized_) {
        return;
    }

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwSetErrorCallback(GLFWErrorCallback);
    glfwInitialized_ = true;
    log::Info("GLFW initialized");
}

void GLFWWindowImpl::TerminateGLFW() {
    if (!glfwInitialized_) {
        return;
    }

    glfwTerminate();
    glfwInitialized_ = false;
    log::Info("GLFW terminated");
}


ref<GLFWWindowImpl> GLFWWindowImpl::CreateFromBuilder(const Window::Builder& builder){
    InitializeGLFW();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, builder.resizable_ ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, builder.decorated_ ? GLFW_TRUE : GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWmonitor* monitor = builder.fullscreen_ ? glfwGetPrimaryMonitor() : nullptr;

    GLFWwindow* glfw_window = glfwCreateWindow(
        static_cast<int>(builder.width_),
        static_cast<int>(builder.height_),
        builder.title_.c_str(),
        monitor,
        nullptr
    );

    if (!glfw_window) {
        if (glfwWindowCount_ == 0) {
            TerminateGLFW();
        }
        throw std::runtime_error("Failed to create GLFW window");
    }

    auto window = ref<GLFWWindowImpl>(new GLFWWindowImpl());
    window->window_ = glfw_window;
    window->title_ = builder.title_;
    window->width_ = builder.width_;
    window->height_ = builder.height_;
    window->vsync_ = builder.vsync_;
    window->fullscreen_ = builder.fullscreen_;
    window->resizable_ = builder.resizable_;
    window->should_close_ = false;

    glfwWindowCount_++;

    glfwSetWindowUserPointer(glfw_window, window.get());

    glfwSetFramebufferSizeCallback(glfw_window, [](GLFWwindow* w, int width, int height) {
        auto* win = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(w));
        if (win) {
            win->width_ = static_cast<u32>(width);
            win->height_ = static_cast<u32>(height);
        }
    });

    glfwSetWindowCloseCallback(glfw_window, [](GLFWwindow* w) {
        auto* win = static_cast<GLFWWindowImpl*>(glfwGetWindowUserPointer(w));
        if (win) {
            win->should_close_ = true;
        }
    });

    log::Info("Window created: '{}' ({}x{})", builder.title_, builder.width_, builder.height_);

    return window;
}


bool GLFWWindowImpl::ShouldClose() const  noexcept  {
    return should_close_ || glfwWindowShouldClose(window_);
}
void GLFWWindowImpl::PollEvents() noexcept {
    glfwPollEvents();
}
void GLFWWindowImpl::Close() noexcept {
    should_close_ = true;
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
}

u32 GLFWWindowImpl::GetWidth() const noexcept {return width_;}
u32 GLFWWindowImpl::GetHeight() const noexcept {return height_;}
std::string_view GLFWWindowImpl::GetTitle() const noexcept {return title_;}

bool GLFWWindowImpl::IsVSync() const noexcept {return vsync_;}
bool GLFWWindowImpl::IsFullscreen() const noexcept {return fullscreen_;}
bool GLFWWindowImpl::IsResizable() const noexcept { return resizable_;}

void GLFWWindowImpl::SetVSync(bool enabled) noexcept {
    vsync_ = enabled;
    if (glfwGetCurrentContext() == window_) {
        glfwSwapInterval(enabled ? 1 : 0);
    }
}
void GLFWWindowImpl::SetTitle(std::string_view title) {
    title_ = title;
    glfwSetWindowTitle(window_, title_.c_str());
}
void GLFWWindowImpl::SetSize(u32 width, u32 height) {
    width_ = width;
    height_ = height;
    glfwSetWindowSize(window_, static_cast<int>(width), static_cast<int>(height));
}

void* GLFWWindowImpl::GetNativeHandle() const noexcept {
    return static_cast<void*>(window_);}


} // namespace cc::gfx
