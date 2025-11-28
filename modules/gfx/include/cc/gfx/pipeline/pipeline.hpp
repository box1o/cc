#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <vector>

namespace cc::gfx {

class Pipeline {
public:
    class Builder {
    public:
        Builder& SetShader(Shader* shader);
        Builder& SetVertexLayout(VertexLayout* layout);
        Builder& AddDescriptorLayout(DescriptorSetLayout* layout);
        Builder& SetPrimitiveTopology(PrimitiveTopology topology);
        Builder& SetCullMode(CullMode mode);
        Builder& SetFrontFace(FrontFace face);
        Builder& SetPolygonMode(PolygonMode mode);
        Builder& SetLineWidth(f32 width);
        Builder& SetDepthTest(bool enable);
        Builder& SetDepthWrite(bool enable);
        Builder& SetDepthCompare(CompareOp op);
        Builder& SetStencilTest(bool enable);
        Builder& SetBlendEnabled(bool enable);
        Builder& SetBlendSrcColor(BlendFactor factor);
        Builder& SetBlendDstColor(BlendFactor factor);
        Builder& SetBlendColorOp(BlendOp op);
        Builder& SetBlendSrcAlpha(BlendFactor factor);
        Builder& SetBlendDstAlpha(BlendFactor factor);
        Builder& SetBlendAlphaOp(BlendOp op);
        [[nodiscard]] scope<Pipeline> Build();

    private:
        Device* device_{nullptr};
        Shader* shader_{nullptr};
        VertexLayout* vertexLayout_{nullptr};
        std::vector<DescriptorSetLayout*> descriptorLayouts_;
        PrimitiveTopology topology_{PrimitiveTopology::TriangleList};
        RasterizerState rasterizer_{};
        DepthStencilState depthStencil_{};
        BlendState blend_{};

        friend class Pipeline;
    };

    virtual ~Pipeline() = default;

    [[nodiscard]] static Builder Create(Device* device);

    [[nodiscard]] Shader* GetShader() const noexcept { return shader_; }
    [[nodiscard]] VertexLayout* GetVertexLayout() const noexcept { return vertexLayout_; }
    [[nodiscard]] const std::vector<DescriptorSetLayout*>& GetDescriptorLayouts() const noexcept { return descriptorLayouts_; }
    [[nodiscard]] PrimitiveTopology GetTopology() const noexcept { return topology_; }
    [[nodiscard]] const RasterizerState& GetRasterizerState() const noexcept { return rasterizer_; }
    [[nodiscard]] const DepthStencilState& GetDepthStencilState() const noexcept { return depthStencil_; }
    [[nodiscard]] const BlendState& GetBlendState() const noexcept { return blend_; }

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

    virtual void BindVertexBuffer(u32 binding, Buffer* buffer, u64 offset = 0) const = 0;
    virtual void BindIndexBuffer(Buffer* buffer, IndexType indexType = IndexType::U32) const = 0;
    virtual void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) const = 0;
    virtual void DrawIndexed(
        u32 indexCount,
        u32 instanceCount = 1,
        u32 firstIndex = 0,
        i32 vertexOffset = 0,
        u32 firstInstance = 0
    ) const = 0;

protected:
    Pipeline(
        Shader* shader,
        VertexLayout* vertexLayout,
        std::vector<DescriptorSetLayout*> descriptorLayouts,
        PrimitiveTopology topology,
        const RasterizerState& rasterizer,
        const DepthStencilState& depthStencil,
        const BlendState& blend
    );

    Shader* shader_{nullptr};
    VertexLayout* vertexLayout_{nullptr};
    std::vector<DescriptorSetLayout*> descriptorLayouts_;
    PrimitiveTopology topology_{PrimitiveTopology::TriangleList};
    RasterizerState rasterizer_{};
    DepthStencilState depthStencil_{};
    BlendState blend_{};
};

class PipelineImpl {
public:
    virtual ~PipelineImpl() = default;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

protected:
    PipelineImpl() = default;
};

} // namespace cc::gfx
