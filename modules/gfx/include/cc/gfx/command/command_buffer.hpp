#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

class CommandBuffer {
public:
    virtual ~CommandBuffer() = default;

    [[nodiscard]] static scope<CommandBuffer> Create(Device* device);

    //NOTE: Recording Lifecycle
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void Submit() = 0;
    virtual void Reset() = 0;

    //NOTE: Render Pass
    virtual void BeginRenderPass(const RenderPassBeginInfo& beginInfo) = 0;
    virtual void BeginRenderPass(
        Framebuffer* framebuffer,
        const ClearValue& colorClear = ClearValue(0.0f, 0.0f, 0.0f, 1.0f),
        const ClearValue& depthClear = ClearValue(1.0f, 0)
    ) = 0;
    virtual void EndRenderPass() = 0;

    //NOTE: Viewport & Scissor
    virtual void SetViewport(
        f32 x,
        f32 y,
        f32 width,
        f32 height,
        f32 minDepth = 0.0f,
        f32 maxDepth = 1.0f
    ) = 0;
    virtual void SetViewport(const Viewport& viewport) = 0;
    virtual void SetScissor(i32 x, i32 y, u32 width, u32 height) = 0;
    virtual void SetScissor(const Scissor& scissor) = 0;

    //NOTE: Pipeline Binding
    virtual void BindPipeline(Pipeline* pipeline) = 0;

    //NOTE: Descriptor Binding
    virtual void BindDescriptorSet(u32 setIndex, DescriptorSet* descriptorSet) = 0;

    //NOTE: Vertex & Index Buffers
    virtual void BindVertexBuffer(u32 binding, Buffer* buffer, u64 offset = 0) = 0;
    virtual void BindIndexBuffer(Buffer* buffer, IndexType indexType = IndexType::U32) = 0;

    //NOTE: Draw Commands
    virtual void Draw(
        u32 vertexCount,
        u32 instanceCount = 1,
        u32 firstVertex = 0,
        u32 firstInstance = 0
    ) = 0;

    virtual void DrawIndexed(
        u32 indexCount,
        u32 instanceCount = 1,
        u32 firstIndex = 0,
        i32 vertexOffset = 0,
        u32 firstInstance = 0
    ) = 0;

    virtual void DrawFullscreenQuad() = 0;

    //NOTE: Utility
    virtual void CopyBuffer(
        Buffer* src,
        Buffer* dst,
        u64 size,
        u64 srcOffset = 0,
        u64 dstOffset = 0
    ) = 0;

    virtual void PushConstants(ShaderStage stage, u32 offset, u32 size, const void* data) = 0;

    //NOTE: State Queries
    [[nodiscard]] virtual bool IsRecording() const = 0;
    [[nodiscard]] virtual bool IsInsideRenderPass() const = 0;

protected:
    CommandBuffer() = default;
};

class CommandBufferImpl {
public:
    virtual ~CommandBufferImpl() = default;

protected:
    CommandBufferImpl() = default;
};

} // namespace cc::gfx
