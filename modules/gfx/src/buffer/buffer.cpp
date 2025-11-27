#include <cc/gfx/buffer/buffer.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>

namespace cc::gfx {

scope<Buffer> Buffer::Create(Device* device, BufferType type, u64 size, BufferUsage usage, const void* data) {
    if (device == nullptr) {
        log::Critical("Device is required to create buffer");
        throw std::runtime_error("Device is null");
    }

    if (size == 0) {
        log::Critical("Buffer size must be greater than 0");
        throw std::runtime_error("Invalid buffer size");
    }

    return device->CreateBuffer(type, size, usage, data);
}

Buffer::Buffer(BufferType type, BufferUsage usage, u64 size, scope<BufferImpl> impl)
    : type_(type)
    , usage_(usage)
    , size_(size)
    , impl_(std::move(impl)) {}

Buffer::~Buffer() = default;

void Buffer::Update(const void* data, u64 size, u64 offset) {
    if (offset + size > size_) {
        log::Error("Buffer update out of bounds: offset={}, size={}, bufferSize={}", offset, size, size_);
        return;
    }
    impl_->Update(data, size, offset);
}

u32 Buffer::GetHandle() const {
    return impl_->GetHandle();
}

} // namespace cc::gfx
