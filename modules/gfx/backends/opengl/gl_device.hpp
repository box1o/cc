#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/gfx/types.hpp>

struct GLFWwindow;

namespace cc::gfx {

class OpenGLDevice final : public Device {
public:
    ~OpenGLDevice() override;

    [[nodiscard]] const DeviceCapabilities& GetCapabilities() const noexcept override { return capabilities_; }
    [[nodiscard]] const DeviceInfo& GetInfo() const noexcept override { return info_; }
    [[nodiscard]] Backend GetBackend() const noexcept override { return Backend::OpenGL; }

    void WaitIdle() override;

    static scope<Device> CreateFromBuilder(const Device::Builder& builder);

    static void DebugCallbackImpl(
        unsigned int source,
        unsigned int type,
        unsigned int id,
        unsigned int severity,
        int length,
        const char* message,
        const void* userParam
    );

private:
    OpenGLDevice() = default;

    void Initialize(GLFWwindow* window, bool enableValidation);
    void QueryCapabilities();
    void QueryInfo();
    static void SetupDebugCallback();

    DeviceCapabilities capabilities_{};
    DeviceInfo info_{};
};

[[nodiscard]] scope<Device> CreateOpenGLDevice(const Device::Builder& builder);

} // namespace cc::gfx
