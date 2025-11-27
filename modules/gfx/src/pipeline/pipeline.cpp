#include <cc/gfx/pipeline/pipeline.hpp>
#include <cc/gfx/pipeline/vertex_layout.hpp>
#include <cc/gfx/descriptor/descriptor_set_layout.hpp>
#include <cc/gfx/shader/shader.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>

namespace cc::gfx {

Pipeline::Builder Pipeline::Create(Device* device) {
    Builder builder;
    builder.device_ = device;
    return builder;
}

Pipeline::Builder& Pipeline::Builder::SetShader(Shader* shader) {
    shader_ = shader;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetVertexLayout(VertexLayout* layout) {
    vertexLayout_ = layout;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::AddDescriptorLayout(DescriptorSetLayout* layout) {
    descriptorLayouts_.push_back(layout);
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetPrimitiveTopology(PrimitiveTopology topology) {
    topology_ = topology;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetCullMode(CullMode mode) {
    rasterizer_.cullMode = mode;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetFrontFace(FrontFace face) {
    rasterizer_.frontFace = face;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetPolygonMode(PolygonMode mode) {
    rasterizer_.polygonMode = mode;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetLineWidth(f32 width) {
    rasterizer_. lineWidth = width;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetDepthTest(bool enable) {
    depthStencil_.depthTestEnable = enable;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetDepthWrite(bool enable) {
    depthStencil_.depthWriteEnable = enable;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetDepthCompare(CompareOp op) {
    depthStencil_. depthCompareOp = op;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetStencilTest(bool enable) {
    depthStencil_.stencilTestEnable = enable;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetBlendEnabled(bool enable) {
    blend_.enabled = enable;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetBlendSrcColor(BlendFactor factor) {
    blend_.srcColorFactor = factor;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetBlendDstColor(BlendFactor factor) {
    blend_. dstColorFactor = factor;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetBlendColorOp(BlendOp op) {
    blend_.colorBlendOp = op;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetBlendSrcAlpha(BlendFactor factor) {
    blend_.srcAlphaFactor = factor;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetBlendDstAlpha(BlendFactor factor) {
    blend_.dstAlphaFactor = factor;
    return *this;
}

Pipeline::Builder& Pipeline::Builder::SetBlendAlphaOp(BlendOp op) {
    blend_.alphaBlendOp = op;
    return *this;
}

scope<Pipeline> Pipeline::Builder::Build() {
    if (device_ == nullptr) {
        log::Critical("Device is required to create Pipeline");
        throw std::runtime_error("Device is null");
    }

    if (shader_ == nullptr) {
        log::Critical("Shader is required to create Pipeline");
        throw std::runtime_error("Shader is null");
    }

    return device_->CreatePipeline(
        shader_,
        vertexLayout_,
        descriptorLayouts_,
        topology_,
        rasterizer_,
        depthStencil_,
        blend_
    );
}

Pipeline::Pipeline(
    Shader* shader,
    VertexLayout* vertexLayout,
    std::vector<DescriptorSetLayout*> descriptorLayouts,
    PrimitiveTopology topology,
    const RasterizerState& rasterizer,
    const DepthStencilState& depthStencil,
    const BlendState& blend
)
    : shader_(shader)
    , vertexLayout_(vertexLayout)
    , descriptorLayouts_(std::move(descriptorLayouts))
    , topology_(topology)
    , rasterizer_(rasterizer)
    , depthStencil_(depthStencil)
    , blend_(blend) {}

} // namespace cc::gfx
