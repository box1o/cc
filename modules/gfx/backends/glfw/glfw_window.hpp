#pragma once
#include <cc/gfx/window/window.hpp>
#include <string>

struct GLFWwindow;

namespace cc::gfx {

class GLFWWindowImpl final : public Window {
public:
    ~GLFWWindowImpl() override;

    static scope<GLFWWindowImpl> Create(const WindowConfig& config);

    bool ShouldClose() const override;
    void PollEvents() override;
    void SwapBuffers() override;
    void Close() override;

    u32 GetWidth() const override;
    u32 GetHeight() const override;
    std::string_view GetTitle() const override;

    bool IsVSync() const override;
    bool IsFullscreen() const override;
    bool IsResizable() const override;

    void SetVSync(bool enabled) override;
    void SetTitle(std::string_view title) override;
    void SetSize(u32 width, u32 height) override;

    void* GetNativeHandle() const override;
    WindowBackend GetBackend() const override { return WindowBackend::GLFW; }

    void SetResizeCallback(WindowResizeCallback callback) override;

private:
    GLFWWindowImpl() = default;

    static void InitGLFW();
    static void TerminateGLFW();
    static void ErrorCallback(int error, const char* description);
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void WindowCloseCallback(GLFWwindow* window);

    GLFWwindow* handle_{nullptr};
    std::string title_;
    u32 width_{0};
    u32 height_{0};
    bool resizable_{true};
    bool vsync_{true};
    bool fullscreen_{false};
    bool decorated_{true};
    
    WindowResizeCallback resizeCallback_;

    static inline u32 s_windowCount{0};
    static inline bool s_initialized{false};
};

} // namespace cc::gfx
