#include <cc/gfx/command/command_buffer.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_command_buffer.hpp"
#include <stdexcept>

namespace cc::gfx {

[[nodiscard]] scope<CommandBuffer> CommandBuffer::Create(Device* device) {
    if (device == nullptr) {
        log::Critical("Device is required to create CommandBuffer");
        throw std::runtime_error("Device is null");
    }

    switch (device->GetBackend()) {
        case Backend::OpenGL:
            return CreateOpenGLCommandBuffer(device);
        case Backend::Vulkan:
            log::Critical("Vulkan command buffer backend not implemented");
            throw std::runtime_error("Vulkan command buffer backend not implemented");
        case Backend::Metal:
            log::Critical("Metal command buffer backend not implemented");
            throw std::runtime_error("Metal command buffer backend not implemented");
    }

    log::Critical("Unknown backend in CommandBuffer::Create");
    throw std::runtime_error("Unknown backend");
}

} // namespace cc::gfx
