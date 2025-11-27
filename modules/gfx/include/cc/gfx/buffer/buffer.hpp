#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

class Buffer {
public:
    ~Buffer();

    static scope<Buffer> Create(Device* device, BufferType type, u64 size, BufferUsage usage = BufferUsage::Static, const void* data = nullptr);

    void Update(const void* data, u64 size, u64 offset = 0);

    BufferType GetType() const { return type_; }
    BufferUsage GetUsage() const { return usage_; }
    u64 GetSize() const { return size_; }
    u32 GetHandle() const;

private:
    Buffer(BufferType type, BufferUsage usage, u64 size, scope<BufferImpl> impl);

    BufferType type_;
    BufferUsage usage_;
    u64 size_;
    scope<BufferImpl> impl_;

    friend scope<Buffer> CreateOpenGLBuffer(Device*, BufferType, u64, BufferUsage, const void*);
};

class BufferImpl {
public:
    virtual ~BufferImpl() = default;
    virtual void Update(const void* data, u64 size, u64 offset) = 0;
    virtual u32 GetHandle() const = 0;

protected:
    BufferImpl() = default;
};

} // namespace cc::gfx
