#pragma once
#include <cc/gfx/window/window.hpp>
#include <cc/gfx/events/bus.hpp>
#include <cc/gfx/events/window/events.hpp>
#include <cc/gfx/events/input/events.hpp>
#include <cc/gfx/events/render/events.hpp>
#include <string>

struct GLFWwindow;

namespace cc::gfx {

class GLFWWindowImpl final : public Window {
public:
    ~GLFWWindowImpl() override;

    [[nodiscard]] static scope<GLFWWindowImpl> Create(const WindowConfig& config);

    [[nodiscard]] bool ShouldClose() const override;
    void PollEvents() override;
    void SwapBuffers() override;
    void Close() override;

    [[nodiscard]] u32 GetWidth() const override;
    [[nodiscard]] u32 GetHeight() const override;
    [[nodiscard]] std::string_view GetTitle() const override;

    [[nodiscard]] bool IsVSync() const override;
    [[nodiscard]] bool IsFullscreen() const override;
    [[nodiscard]] bool IsResizable() const override;

    void SetVSync(bool enabled) override;
    void SetTitle(std::string_view title) override;
    void SetSize(u32 width, u32 height) override;

    [[nodiscard]] void* GetNativeHandle() const override;
    [[nodiscard]] WindowBackend GetBackend() const override { return WindowBackend::GLFW; }

    void SetResizeCallback(WindowResizeCallback callback) override;
    void SetEventBus(const ref<events::EventBus>& bus) override;

private:
    GLFWWindowImpl() = default;

    static void InitGLFW();
    static void TerminateGLFW();
    static void ErrorCallback(int error, const char* description);

    // GLFW callbacks
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void WindowCloseCallback(GLFWwindow* window);
    static void WindowFocusCallback(GLFWwindow* window, int focused);
    static void WindowPosCallback(GLFWwindow* window, int xpos, int ypos);
    static void WindowIconifyCallback(GLFWwindow* window, int iconified);
    static void WindowMaximizeCallback(GLFWwindow* window, int maximized);

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CharCallback(GLFWwindow* window, unsigned int codepoint);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void CursorEnterCallback(GLFWwindow* window, int entered);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    GLFWwindow* handle_{nullptr};
    std::string title_;
    u32 width_{0};
    u32 height_{0};
    bool resizable_{true};
    bool vsync_{true};
    bool fullscreen_{false};
    bool decorated_{true};

    WindowResizeCallback resizeCallback_{};

    ref<events::EventBus> eventBus_{};

    double lastCursorX_{0.0};
    double lastCursorY_{0.0};
    bool   hasLastCursor_{false};

    static inline u32 s_windowCount{0};
    static inline bool s_initialized{false};
};

} // namespace cc::gfx
