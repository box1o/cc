#pragma once
#include "window/window.hpp"
#include "types.hpp"
#include "GLFW/glfw3.h"


namespace cc::gfx {

class GLFWWindowImpl final : public Window{
public:
    ~GLFWWindowImpl() override;
    static ref<GLFWWindowImpl> CreateFromBuilder(const Window::Builder& builder);


    // NOTE: interface implementation
    [[nodiscard]]  bool ShouldClose() const  noexcept  override;
    void PollEvents() noexcept override;
    void Close() noexcept override;

    [[nodiscard]]  u32 GetWidth() const noexcept override;
    [[nodiscard]]  u32 GetHeight() const noexcept override;
    [[nodiscard]]  std::string_view GetTitle() const noexcept override;

    [[nodiscard]]  bool IsVSync() const noexcept override;
    [[nodiscard]]  bool IsFullscreen() const noexcept override;
    [[nodiscard]]  bool IsResizable() const noexcept override;

    void SetVSync(bool enabled) noexcept override;
    void SetTitle(std::string_view title) override;
    void SetSize(u32 width, u32 height) override;

    void* GetNativeHandle() const noexcept override;

private:
    GLFWWindowImpl() = default;

    static void InitializeGLFW();
    static void TerminateGLFW();

    GLFWwindow* window_{nullptr};
    std::string title_{"studio"};
    u32 width_{1920};
    u32 height_{1080};
    bool resizable_{true};
    bool vsync_{true};
    bool fullscreen_{false};
    bool decorated_{true};
    bool should_close_{false};

    static inline u32 glfwWindowCount_{0};
    static inline bool glfwInitialized_{false};


    friend class Window::Builder;

};


} // namespace cc::gfx
