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
    u32 GetHandle() const override { return handle_; }

private:
    unsigned int GetGLTarget() const;
    unsigned int GetGLUsage() const;

    u32 handle_{0};
    BufferType type_;
    BufferUsage usage_;
    u64 size_;
};

scope<Buffer> CreateOpenGLBuffer(Device* device, BufferType type, u64 size, BufferUsage usage, const void* data);

} // namespace cc::gfx
