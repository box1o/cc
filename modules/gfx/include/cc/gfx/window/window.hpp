#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <string>
#include <string_view>
#include <functional>

namespace cc::gfx::events {
class EventBus;
}

namespace cc::gfx {

using WindowResizeCallback = std::function<void(u32 width, u32 height)>;

struct WindowConfig {
    std::string title{"Window"};
    u32 width{1280};
    u32 height{720};
    bool resizable{true};
    bool vsync{true};
    bool fullscreen{false};
    bool decorated{true};
    WindowBackend backend{WindowBackend::GLFW};
};

class Window {
public:
    class Builder {
    public:
        [[nodiscard]] Builder& SetTitle(std::string_view title) noexcept;
        [[nodiscard]] Builder& SetSize(u32 width, u32 height) noexcept;
        [[nodiscard]] Builder& SetResizable(bool resizable) noexcept;
        [[nodiscard]] Builder& SetVSync(bool vsync) noexcept;
        [[nodiscard]] Builder& SetFullscreen(bool fullscreen) noexcept;
        [[nodiscard]] Builder& SetDecorated(bool decorated) noexcept;
        [[nodiscard]] Builder& SetBackend(WindowBackend backend) noexcept;
        [[nodiscard]] scope<Window> Build();

    private:
        WindowConfig config_{};
        friend class Window;
    };

    virtual ~Window() = default;

    [[nodiscard]] static Builder Create() noexcept;

    [[nodiscard]] virtual bool ShouldClose() const = 0;
    virtual void PollEvents() = 0;
    virtual void SwapBuffers() = 0;
    virtual void Close() = 0;

    [[nodiscard]] virtual u32 GetWidth() const = 0;
    [[nodiscard]] virtual u32 GetHeight() const = 0;
    [[nodiscard]] virtual std::string_view GetTitle() const = 0;

    [[nodiscard]] virtual bool IsVSync() const = 0;
    [[nodiscard]] virtual bool IsFullscreen() const = 0;
    [[nodiscard]] virtual bool IsResizable() const = 0;

    virtual void SetVSync(bool enabled) = 0;
    virtual void SetTitle(std::string_view title) = 0;
    virtual void SetSize(u32 width, u32 height) = 0;

    [[nodiscard]] virtual void* GetNativeHandle() const = 0;
    [[nodiscard]] virtual WindowBackend GetBackend() const = 0;

    virtual void SetResizeCallback(WindowResizeCallback callback) = 0;

    //NOTE: attach an EventBus instance to the window
    virtual void SetEventBus(const ref<events::EventBus>& bus) = 0;

    [[nodiscard]] f32 GetAspectRatio() const {
        const u32 h = GetHeight();
        return h > 0 ? static_cast<f32>(GetWidth()) / static_cast<f32>(h) : 1.0f;
    }

protected:
    Window() = default;
};

} // namespace cc::gfx
