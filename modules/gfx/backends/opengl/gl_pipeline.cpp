#include "gl_pipeline.hpp"
#include <cc/gfx/pipeline/vertex_layout.hpp>
#include <cc/gfx/buffer/buffer.hpp>
#include <cc/gfx/shader/shader.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>

namespace cc::gfx {

namespace {

struct GLVertexFormatInfo {
    int componentCount;
    unsigned int type;
    bool normalized;
};

[[nodiscard]] GLVertexFormatInfo GetGLVertexFormat(VertexFormat format) noexcept {
    switch (format) {
        case VertexFormat::Float:      return {1, GL_FLOAT,          false};
        case VertexFormat::Float2:     return {2, GL_FLOAT,          false};
        case VertexFormat::Float3:     return {3, GL_FLOAT,          false};
        case VertexFormat::Float4:     return {4, GL_FLOAT,          false};
        case VertexFormat::Int:        return {1, GL_INT,            false};
        case VertexFormat::Int2:       return {2, GL_INT,            false};
        case VertexFormat::Int3:       return {3, GL_INT,            false};
        case VertexFormat::Int4:       return {4, GL_INT,            false};
        case VertexFormat::UInt:       return {1, GL_UNSIGNED_INT,   false};
        case VertexFormat::UInt2:      return {2, GL_UNSIGNED_INT,   false};
        case VertexFormat::UInt3:      return {3, GL_UNSIGNED_INT,   false};
        case VertexFormat::UInt4:      return {4, GL_UNSIGNED_INT,   false};
        case VertexFormat::Byte4Norm:  return {4, GL_BYTE,           true};
        case VertexFormat::UByte4Norm: return {4, GL_UNSIGNED_BYTE,  true};
    }
    return {4, GL_FLOAT, false};
}

} // anonymous namespace

OpenGLPipeline::OpenGLPipeline(
    Shader* shader,
    VertexLayout* vertexLayout,
    std::vector<DescriptorSetLayout*> descriptorLayouts,
    PrimitiveTopology topology,
    const RasterizerState& rasterizer,
    const DepthStencilState& depthStencil,
    const BlendState& blend
)
    : Pipeline(shader, vertexLayout, std::move(descriptorLayouts), topology, rasterizer, depthStencil, blend)
    , handle_(s_nextHandle++) {

    CreateVAO();

    log::Info("OpenGL Pipeline created (handle={}, vao={})", handle_, vao_);
}

OpenGLPipeline::~OpenGLPipeline() {
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        log::Trace("OpenGL Pipeline destroyed (handle={}, vao={})", handle_, vao_);
        vao_ = 0;
    }
}

void OpenGLPipeline::CreateVAO() {
    glCreateVertexArrays(1, &vao_);

    if (vao_ == 0) {
        log::Critical("Failed to create VAO");
        return;
    }

    if (vertexLayout_ == nullptr) {
        log::Trace("Pipeline created without vertex layout (fullscreen or compute pass)");
        return;
    }

    for (const auto& binding : vertexLayout_->GetBindings()) {
        glVertexArrayBindingDivisor(
            vao_,
            binding.binding,
            binding.inputRate == VertexInputRate::PerInstance ? 1 : 0
        );
    }

    for (const auto& attr : vertexLayout_->GetAttributes()) {
        const GLVertexFormatInfo formatInfo = GetGLVertexFormat(attr.format);

        glEnableVertexArrayAttrib(vao_, attr.location);

        if (formatInfo.type == GL_INT || formatInfo.type == GL_UNSIGNED_INT) {
            glVertexArrayAttribIFormat(
                vao_,
                attr.location,
                formatInfo.componentCount,
                formatInfo.type,
                attr.offset
            );
        } else {
            glVertexArrayAttribFormat(
                vao_,
                attr.location,
                formatInfo.componentCount,
                formatInfo.type,
                formatInfo.normalized ? GL_TRUE : GL_FALSE,
                attr.offset
            );
        }

        glVertexArrayAttribBinding(vao_, attr.location, attr.binding);
    }
}

void OpenGLPipeline::Bind() const {
    if (shader_ != nullptr) {
        shader_->Bind();
    }

    glBindVertexArray(vao_);

    ApplyRasterizerState();
    ApplyDepthStencilState();
    ApplyBlendState();
}

void OpenGLPipeline::Unbind() const {
    glBindVertexArray(0);

    if (shader_ != nullptr) {
        shader_->Unbind();
    }
}

void OpenGLPipeline::BindVertexBuffer(u32 binding, Buffer* buffer, u64 offset) const {
    if (buffer == nullptr) {
        log::Warn("Attempting to bind null vertex buffer");
        return;
    }

    if (vertexLayout_ == nullptr) {
        log::Warn("Cannot bind vertex buffer without vertex layout");
        return;
    }

    const u32 stride = vertexLayout_->GetStride(binding);

    glVertexArrayVertexBuffer(
        vao_,
        binding,
        buffer->GetHandle(),
        static_cast<GLintptr>(offset),
        static_cast<GLsizei>(stride)
    );
}

void OpenGLPipeline::BindIndexBuffer(Buffer* buffer, IndexType indexType) const {
    if (buffer == nullptr) {
        log::Warn("Attempting to bind null index buffer");
        return;
    }

    boundIndexType_ = indexType;
    glVertexArrayElementBuffer(vao_, buffer->GetHandle());
}

void OpenGLPipeline::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) const {
    const unsigned int glTopology = GetGLTopology(topology_);

    if (instanceCount > 1 || firstInstance > 0) {
        glDrawArraysInstancedBaseInstance(
            glTopology,
            static_cast<GLint>(firstVertex),
            static_cast<GLsizei>(vertexCount),
            static_cast<GLsizei>(instanceCount),
            firstInstance
        );
    } else {
        glDrawArrays(
            glTopology,
            static_cast<GLint>(firstVertex),
            static_cast<GLsizei>(vertexCount)
        );
    }
}

void OpenGLPipeline::DrawIndexed(
    u32 indexCount,
    u32 instanceCount,
    u32 firstIndex,
    i32 vertexOffset,
    u32 firstInstance
) const {
    const unsigned int glTopology  = GetGLTopology(topology_);
    const unsigned int glIndexType = (boundIndexType_ == IndexType::U16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
    const size_t indexSize         = (boundIndexType_ == IndexType::U16) ? sizeof(u16) : sizeof(u32);

    void* offset = reinterpret_cast<void*>(firstIndex * indexSize);

    if (instanceCount > 1 || firstInstance > 0 || vertexOffset != 0) {
        glDrawElementsInstancedBaseVertexBaseInstance(
            glTopology,
            static_cast<GLsizei>(indexCount),
            glIndexType,
            offset,
            static_cast<GLsizei>(instanceCount),
            vertexOffset,
            firstInstance
        );
    } else {
        glDrawElements(
            glTopology,
            static_cast<GLsizei>(indexCount),
            glIndexType,
            offset
        );
    }
}

void OpenGLPipeline::ApplyRasterizerState() const {
    if (rasterizer_.cullMode == CullMode::None) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace(rasterizer_.cullMode == CullMode::Front ? GL_FRONT : GL_BACK);
    }

    glFrontFace(rasterizer_.frontFace == FrontFace::Clockwise ? GL_CW : GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GetGLPolygonMode(rasterizer_.polygonMode));
    glLineWidth(rasterizer_.lineWidth);

    if (rasterizer_.depthClampEnable) {
        glEnable(GL_DEPTH_CLAMP);
    } else {
        glDisable(GL_DEPTH_CLAMP);
    }
}

void OpenGLPipeline::ApplyDepthStencilState() const {
    if (depthStencil_.depthTestEnable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GetGLCompareOp(depthStencil_.depthCompareOp));
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    glDepthMask(depthStencil_.depthWriteEnable ? GL_TRUE : GL_FALSE);

    if (depthStencil_.stencilTestEnable) {
        glEnable(GL_STENCIL_TEST);
    } else {
        glDisable(GL_STENCIL_TEST);
    }
}

void OpenGLPipeline::ApplyBlendState() const {
    if (blend_.enabled) {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(
            GetGLBlendFactor(blend_.srcColorFactor),
            GetGLBlendFactor(blend_.dstColorFactor),
            GetGLBlendFactor(blend_.srcAlphaFactor),
            GetGLBlendFactor(blend_.dstAlphaFactor)
        );
        glBlendEquationSeparate(
            GetGLBlendOp(blend_.colorBlendOp),
            GetGLBlendOp(blend_.alphaBlendOp)
        );
    } else {
        glDisable(GL_BLEND);
    }
}

unsigned int OpenGLPipeline::GetGLTopology(PrimitiveTopology topology) noexcept {
    switch (topology) {
        case PrimitiveTopology::PointList:     return GL_POINTS;
        case PrimitiveTopology::LineList:      return GL_LINES;
        case PrimitiveTopology::LineStrip:     return GL_LINE_STRIP;
        case PrimitiveTopology::TriangleList:  return GL_TRIANGLES;
        case PrimitiveTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
    }
    return GL_TRIANGLES;
}

unsigned int OpenGLPipeline::GetGLCompareOp(CompareOp op) noexcept {
    switch (op) {
        case CompareOp::Never:        return GL_NEVER;
        case CompareOp::Less:         return GL_LESS;
        case CompareOp::Equal:        return GL_EQUAL;
        case CompareOp::LessEqual:    return GL_LEQUAL;
        case CompareOp::Greater:      return GL_GREATER;
        case CompareOp::NotEqual:     return GL_NOTEQUAL;
        case CompareOp::GreaterEqual: return GL_GEQUAL;
        case CompareOp::Always:       return GL_ALWAYS;
    }
    return GL_LESS;
}

unsigned int OpenGLPipeline::GetGLBlendFactor(BlendFactor factor) noexcept {
    switch (factor) {
        case BlendFactor::Zero:             return GL_ZERO;
        case BlendFactor::One:              return GL_ONE;
        case BlendFactor::SrcColor:         return GL_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:         return GL_DST_COLOR;
        case BlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:         return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:         return GL_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
    }
    return GL_ONE;
}

unsigned int OpenGLPipeline::GetGLBlendOp(BlendOp op) noexcept {
    switch (op) {
        case BlendOp::Add:             return GL_FUNC_ADD;
        case BlendOp::Subtract:        return GL_FUNC_SUBTRACT;
        case BlendOp::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
        case BlendOp::Min:             return GL_MIN;
        case BlendOp::Max:             return GL_MAX;
    }
    return GL_FUNC_ADD;
}

unsigned int OpenGLPipeline::GetGLPolygonMode(PolygonMode mode) noexcept {
    switch (mode) {
        case PolygonMode::Fill:  return GL_FILL;
        case PolygonMode::Line:  return GL_LINE;
        case PolygonMode::Point: return GL_POINT;
    }
    return GL_FILL;
}

[[nodiscard]] scope<Pipeline> CreateOpenGLPipeline(
    Device* /*device*/,
    Shader* shader,
    VertexLayout* vertexLayout,
    const std::vector<DescriptorSetLayout*>& descriptorLayouts,
    PrimitiveTopology topology,
    const RasterizerState& rasterizer,
    const DepthStencilState& depthStencil,
    const BlendState& blend
) {
    return scope<Pipeline>(new OpenGLPipeline(
        shader,
        vertexLayout,
        descriptorLayouts,
        topology,
        rasterizer,
        depthStencil,
        blend
    ));
}

} // namespace cc::gfx
