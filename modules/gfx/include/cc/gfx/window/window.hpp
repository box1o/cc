#pragma once
#include <cc/core/core.hpp>

namespace cc::gfx {

enum class WindowBackend :u8{
    GLFW = 0,
    NATIVE = 1,
    // ... for ios, android later
};


class Window{
public:
    class Builder{
    public:
        Builder() = default;

        Builder& SetTitle(std::string_view title){
            title_ = title;
            return *this;
        }
        Builder& SetSize(u32 width, u32 height){
            width_ = width;
            height_ = height;
            return *this;
        }
        Builder& SetResizable(bool resizable) {
            resizable_ = resizable;
            return *this;
        }
        Builder& SetVSync(bool vsync) {
            vsync_ = vsync;
            return *this;
        }
        Builder& SetFullscreen(bool fullscreen) {
            fullscreen_ = fullscreen;
            return *this;
        }
        Builder& SetDecorated(bool decorated) {
            decorated_ = decorated;
            return *this;
        }

        ref<Window> Build();

    private:
        std::string title_{"Studio"};
        u32 width_{1920};
        u32 height_{1080};
        bool resizable_{true};
        bool vsync_{true};
        bool fullscreen_{false};
        bool decorated_{true};

        friend class GLFWWindowImpl;  

    };

    // NOTE: interface
    virtual ~Window() = default;



    static Builder Create(WindowBackend backend = WindowBackend::GLFW);

    [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;
    virtual void PollEvents() noexcept = 0;
    virtual void Close() noexcept = 0;


    [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
    [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;
    [[nodiscard]] virtual std::string_view GetTitle() const noexcept = 0;

    [[nodiscard]] virtual bool IsVSync() const noexcept = 0;
    [[nodiscard]] virtual bool IsFullscreen() const noexcept = 0;
    [[nodiscard]] virtual bool IsResizable() const noexcept = 0;

    virtual void SetVSync(bool enabled) noexcept = 0;
    virtual void SetTitle(std::string_view title) = 0;
    virtual void SetSize(u32 width, u32 height) = 0;

    virtual void* GetNativeHandle() const noexcept = 0;

    f32 GetAspectRatio() const noexcept {
        return static_cast<f32>(GetWidth()) / static_cast<f32>(GetHeight());
    }



protected:
    Window() = default;

};

} // namespace cc::gfx
