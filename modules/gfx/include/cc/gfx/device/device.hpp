#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

class Device {
public:
    class Builder {
    public:
        [[nodiscard]] Builder& SetBackend(Backend backend) noexcept;
        [[nodiscard]] Builder& EnableValidation(bool enable) noexcept;
        [[nodiscard]] scope<Device> Build();

    private:
        Backend backend_{Backend::OpenGL};
        bool enableValidation_{false};
        Window* window_{nullptr};

        friend class Device;
        friend class OpenGLDevice;
    };

    virtual ~Device() = default;

    [[nodiscard]] static Builder Create(Window* window, Backend backend = Backend::OpenGL) noexcept;

    [[nodiscard]] virtual const DeviceCapabilities& GetCapabilities() const noexcept = 0;
    [[nodiscard]] virtual const DeviceInfo& GetInfo() const noexcept = 0;
    [[nodiscard]] virtual Backend GetBackend() const noexcept = 0;

    virtual void WaitIdle() = 0;

protected:
    Device() = default;
};

} // namespace cc::gfx
