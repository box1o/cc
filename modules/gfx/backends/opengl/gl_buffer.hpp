#pragma once
#include <cc/gfx/buffer/buffer.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>

namespace cc::gfx {

class Device;

class OpenGLBufferImpl final : public BufferImpl {
public:
    OpenGLBufferImpl(BufferType type, BufferUsage usage, u64 size, const void* data);
    ~OpenGLBufferImpl() override;

    void Update(const void* data, u64 size, u64 offset) override;
    [[nodiscard]] u32 GetHandle() const noexcept override { return handle_; }

private:
    [[nodiscard]] unsigned int GetGLTarget() const noexcept;
    [[nodiscard]] unsigned int GetGLUsage() const noexcept;

    u32 handle_{0};
    BufferType type_{BufferType::Vertex};
    BufferUsage usage_{BufferUsage::Static};
    u64 size_{0};
};

[[nodiscard]] scope<Buffer> CreateOpenGLBuffer(
    Device* device,
    BufferType type,
    u64 size,
    BufferUsage usage,
    const void* data
);

} // namespace cc::gfx
