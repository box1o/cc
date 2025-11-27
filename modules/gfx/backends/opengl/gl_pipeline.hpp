#pragma once
#include <cc/gfx/pipeline/pipeline.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>

namespace cc::gfx {

class Device;

class OpenGLPipeline final : public Pipeline {
public:
    OpenGLPipeline(
        Shader* shader,
        VertexLayout* vertexLayout,
        std::vector<DescriptorSetLayout*> descriptorLayouts,
        PrimitiveTopology topology,
        const RasterizerState& rasterizer,
        const DepthStencilState& depthStencil,
        const BlendState& blend
    );
    ~OpenGLPipeline() override;

    void Bind() const override;
    void Unbind() const override;
    u32 GetHandle() const override { return handle_; }

    void BindVertexBuffer(u32 binding, Buffer* buffer, u64 offset = 0) const override;
    void BindIndexBuffer(Buffer* buffer, IndexType indexType = IndexType::U32) const override;
    void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) const override;
    void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0) const override;

private:
    void CreateVAO();
    void ApplyRasterizerState() const;
    void ApplyDepthStencilState() const;
    void ApplyBlendState() const;

    static unsigned int GetGLTopology(PrimitiveTopology topology);
    static unsigned int GetGLCompareOp(CompareOp op);
    static unsigned int GetGLBlendFactor(BlendFactor factor);
    static unsigned int GetGLBlendOp(BlendOp op);
    static unsigned int GetGLPolygonMode(PolygonMode mode);

    u32 handle_{0};
    u32 vao_{0};
    mutable IndexType boundIndexType_{IndexType::U32};

    static inline u32 s_nextHandle{1};
};

scope<Pipeline> CreateOpenGLPipeline(
    Device* device,
    Shader* shader,
    VertexLayout* vertexLayout,
    const std::vector<DescriptorSetLayout*>& descriptorLayouts,
    PrimitiveTopology topology,
    const RasterizerState& rasterizer,
    const DepthStencilState& depthStencil,
    const BlendState& blend
);

} // namespace cc::gfx
