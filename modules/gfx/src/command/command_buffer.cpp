#include <cc/gfx/command/command_buffer.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>

namespace cc::gfx {

scope<CommandBuffer> CommandBuffer::Create(Device* device) {
    if (device == nullptr) {
        log::Critical("Device is required to create CommandBuffer");
        throw std::runtime_error("Device is null");
    }

    return device->CreateCommandBuffer();
}

} // namespace cc::gfx
