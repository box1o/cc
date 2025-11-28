#pragma once
#include <cc/gfx/command/command_buffer.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>
#include <array>

namespace cc::gfx {

class Device;

class OpenGLCommandBuffer final : public CommandBuffer {
public:
    explicit OpenGLCommandBuffer(Device* device);
    ~OpenGLCommandBuffer() override;

    //NOTE: Recording Lifecycle
    void Begin() override;
    void End() override;
    void Submit() override;
    void Reset() override;

    //NOTE: Render Pass
    void BeginRenderPass(const RenderPassBeginInfo& beginInfo) override;
    void BeginRenderPass(Framebuffer* framebuffer, const ClearValue& colorClear, const ClearValue& depthClear) override;
    void EndRenderPass() override;

    //NOTE: Viewport & Scissor
    void SetViewport(f32 x, f32 y, f32 width, f32 height, f32 minDepth, f32 maxDepth) override;
    void SetViewport(const Viewport& viewport) override;
    void SetScissor(i32 x, i32 y, u32 width, u32 height) override;
    void SetScissor(const Scissor& scissor) override;

    //NOTE: Pipeline Binding
    void BindPipeline(Pipeline* pipeline) override;

    //NOTE: Descriptor Binding
    void BindDescriptorSet(u32 setIndex, DescriptorSet* descriptorSet) override;

    //NOTE: Vertex & Index Buffers
    void BindVertexBuffer(u32 binding, Buffer* buffer, u64 offset) override;
    void BindIndexBuffer(Buffer* buffer, IndexType indexType) override;

    //NOTE: Draw Commands
    void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) override;
    void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) override;
    void DrawFullscreenQuad() override;

    //NOTE: Utility
    void CopyBuffer(Buffer* src, Buffer* dst, u64 size, u64 srcOffset, u64 dstOffset) override;
    void PushConstants(ShaderStage stage, u32 offset, u32 size, const void* data) override;

    //NOTE: State Queries
    bool IsRecording() const override { return isRecording_; }
    bool IsInsideRenderPass() const override { return isInsideRenderPass_; }

private:
    void ApplyPipelineState();
    void CreateFullscreenQuadVAO();

    struct BoundState {
        Pipeline* pipeline{nullptr};
        Framebuffer* framebuffer{nullptr};
        Buffer* indexBuffer{nullptr};
        IndexType indexType{IndexType::U32};
        std::array<DescriptorSet*, MAX_DESCRIPTOR_SETS> descriptorSets{};
        std::array<Buffer*, MAX_VERTEX_BINDINGS> vertexBuffers{};
        std::array<u64, MAX_VERTEX_BINDINGS> vertexOffsets{};
        Viewport viewport{};
        Scissor scissor{};
    };

    Device* device_{nullptr};
    BoundState state_{};
    bool isRecording_{false};
    bool isInsideRenderPass_{false};
    u32 fullscreenQuadVAO_{0};
};

scope<CommandBuffer> CreateOpenGLCommandBuffer(Device* device);

} // namespace cc::gfx
