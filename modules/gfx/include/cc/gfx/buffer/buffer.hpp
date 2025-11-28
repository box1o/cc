#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

class Buffer {
public:
    ~Buffer();

    [[nodiscard]] static scope<Buffer> Create(
        Device* device,
        BufferType type,
        u64 size,
        BufferUsage usage = BufferUsage::Static,
        const void* data = nullptr
    );

    void Update(const void* data, u64 size, u64 offset = 0);

    [[nodiscard]] BufferType GetType() const noexcept { return type_; }
    [[nodiscard]] BufferUsage GetUsage() const noexcept { return usage_; }
    [[nodiscard]] u64 GetSize() const noexcept { return size_; }
    [[nodiscard]] u32 GetHandle() const noexcept;

private:
    Buffer(BufferType type, BufferUsage usage, u64 size, scope<BufferImpl> impl) noexcept;

    BufferType type_{BufferType::Vertex};
    BufferUsage usage_{BufferUsage::Static};
    u64 size_{0};
    scope<BufferImpl> impl_;

    friend scope<Buffer> CreateOpenGLBuffer(Device*, BufferType, u64, BufferUsage, const void*);
};

class BufferImpl {
public:
    virtual ~BufferImpl() = default;
    virtual void Update(const void* data, u64 size, u64 offset) = 0;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

protected:
    BufferImpl() = default;
};

} // namespace cc::gfx
