#include "gl_command_buffer.hpp"
#include <cc/gfx/device/device.hpp>
#include <cc/gfx/pipeline/pipeline.hpp>
#include <cc/gfx/buffer/buffer.hpp>
#include <cc/gfx/framebuffer/framebuffer.hpp>
#include <cc/gfx/descriptor/descriptor_set.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>

namespace cc::gfx {

OpenGLCommandBuffer::OpenGLCommandBuffer(Device* device)
    : device_(device) {
    CreateFullscreenQuadVAO();
    log::Trace("OpenGL CommandBuffer created");
}

OpenGLCommandBuffer::~OpenGLCommandBuffer() {
    if (fullscreenQuadVAO_ != 0) {
        glDeleteVertexArrays(1, &fullscreenQuadVAO_);
        fullscreenQuadVAO_ = 0;
    }
    log::Trace("OpenGL CommandBuffer destroyed");
}

void OpenGLCommandBuffer::CreateFullscreenQuadVAO() {
    glCreateVertexArrays(1, &fullscreenQuadVAO_);
    if (fullscreenQuadVAO_ == 0) {
        log::Error("Failed to create fullscreen quad VAO");
    }
}

void OpenGLCommandBuffer::Begin() {
    if (isRecording_) {
        log::Warn("CommandBuffer::Begin called while already recording");
        return;
    }

    Reset();
    isRecording_ = true;
}

void OpenGLCommandBuffer::End() {
    if (!isRecording_) {
        log::Warn("CommandBuffer::End called while not recording");
        return;
    }

    if (isInsideRenderPass_) {
        log::Warn("CommandBuffer::End called while inside render pass, forcing EndRenderPass");
        EndRenderPass();
    }

    isRecording_ = false;
}

void OpenGLCommandBuffer::Submit() {
    if (isRecording_) {
        log::Warn("CommandBuffer::Submit called while still recording, forcing End");
        End();
    }

    glFlush();
}

void OpenGLCommandBuffer::Reset() {
    state_ = BoundState{};
    isRecording_ = false;
    isInsideRenderPass_ = false;
}

void OpenGLCommandBuffer::BeginRenderPass(const RenderPassBeginInfo& beginInfo) {
    if (!isRecording_) {
        log::Error("Cannot begin render pass: CommandBuffer not recording");
        return;
    }

    if (isInsideRenderPass_) {
        log::Warn("BeginRenderPass called while already inside render pass");
        EndRenderPass();
    }

    state_.framebuffer = beginInfo.framebuffer;

    if (state_.framebuffer != nullptr) {
        state_.framebuffer->Bind();
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    SetViewport(beginInfo.viewport);
    SetScissor(beginInfo.scissor);

    GLbitfield clearMask = 0;

    for (u32 i = 0; i < beginInfo.colorAttachmentCount; ++i) {
        const auto& attachment = beginInfo.colorAttachments[i];
        if (attachment.loadOp == LoadOp::Clear) {
            glClearColor(
                attachment.clearValue.color.r,
                attachment.clearValue.color.g,
                attachment.clearValue.color.b,
                attachment.clearValue.color.a
            );
            clearMask |= GL_COLOR_BUFFER_BIT;
        }
    }

    if (beginInfo.depthAttachment != nullptr) {
        const auto& depth = *beginInfo.depthAttachment;
        if (depth.loadOp == LoadOp::Clear) {
            glClearDepth(static_cast<double>(depth.clearValue.depthStencil.depth));
            clearMask |= GL_DEPTH_BUFFER_BIT;
        }
        if (depth.stencilLoadOp == LoadOp::Clear) {
            glClearStencil(static_cast<int>(depth.clearValue.depthStencil.stencil));
            clearMask |= GL_STENCIL_BUFFER_BIT;
        }
    }

    if (clearMask != 0) {
        glClear(clearMask);
    }

    isInsideRenderPass_ = true;
}

void OpenGLCommandBuffer::BeginRenderPass(
    Framebuffer* framebuffer,
    const ClearValue& colorClear,
    const ClearValue& depthClear
) {
    if (!isRecording_) {
        log::Error("Cannot begin render pass: CommandBuffer not recording");
        return;
    }

    if (isInsideRenderPass_) {
        log::Warn("BeginRenderPass called while already inside render pass");
        EndRenderPass();
    }

    state_.framebuffer = framebuffer;

    if (framebuffer != nullptr) {
        framebuffer->Bind();

        state_.viewport = Viewport{
            0.0f,
            0.0f,
            static_cast<f32>(framebuffer->GetWidth()),
            static_cast<f32>(framebuffer->GetHeight()),
            0.0f,
            1.0f
        };
        state_.scissor = Scissor{
            0,
            0,
            framebuffer->GetWidth(),
            framebuffer->GetHeight()
        };
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    SetViewport(state_.viewport);
    SetScissor(state_.scissor);

    glClearColor(
        colorClear.color.r,
        colorClear.color.g,
        colorClear.color.b,
        colorClear.color.a
    );
    glClearDepth(static_cast<double>(depthClear.depthStencil.depth));
    glClearStencil(static_cast<int>(depthClear.depthStencil.stencil));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    isInsideRenderPass_ = true;
}

void OpenGLCommandBuffer::EndRenderPass() {
    if (!isInsideRenderPass_) {
        log::Warn("EndRenderPass called while not inside render pass");
        return;
    }

    if (state_.framebuffer != nullptr) {
        state_.framebuffer->Unbind();
    }

    state_.framebuffer = nullptr;
    isInsideRenderPass_ = false;
}

void OpenGLCommandBuffer::SetViewport(
    f32 x,
    f32 y,
    f32 width,
    f32 height,
    f32 minDepth,
    f32 maxDepth
) {
    state_.viewport = Viewport{x, y, width, height, minDepth, maxDepth};

    glViewport(
        static_cast<GLint>(x),
        static_cast<GLint>(y),
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height)
    );

    glDepthRange(
        static_cast<double>(minDepth),
        static_cast<double>(maxDepth)
    );
}

void OpenGLCommandBuffer::SetViewport(const Viewport& viewport) {
    SetViewport(
        viewport.x,
        viewport.y,
        viewport.width,
        viewport.height,
        viewport.minDepth,
        viewport.maxDepth
    );
}

void OpenGLCommandBuffer::SetScissor(i32 x, i32 y, u32 width, u32 height) {
    state_.scissor = Scissor{x, y, width, height};

    glEnable(GL_SCISSOR_TEST);
    glScissor(
        x,
        y,
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height)
    );
}

void OpenGLCommandBuffer::SetScissor(const Scissor& scissor) {
    SetScissor(scissor.x, scissor.y, scissor.width, scissor.height);
}

void OpenGLCommandBuffer::BindPipeline(Pipeline* pipeline) {
    if (pipeline == nullptr) {
        log::Warn("BindPipeline called with null pipeline");
        return;
    }

    state_.pipeline = pipeline;
    pipeline->Bind();
}

void OpenGLCommandBuffer::BindDescriptorSet(u32 setIndex, DescriptorSet* descriptorSet) {
    if (setIndex >= MAX_DESCRIPTOR_SETS) {
        log::Error("BindDescriptorSet: setIndex {} exceeds maximum {}", setIndex, MAX_DESCRIPTOR_SETS);
        return;
    }

    if (descriptorSet == nullptr) {
        log::Warn("BindDescriptorSet called with null descriptor set");
        return;
    }

    state_.descriptorSets[setIndex] = descriptorSet;
    descriptorSet->Bind(setIndex);
}

void OpenGLCommandBuffer::BindVertexBuffer(u32 binding, Buffer* buffer, u64 offset) {
    if (binding >= MAX_VERTEX_BINDINGS) {
        log::Error("BindVertexBuffer: binding {} exceeds maximum {}", binding, MAX_VERTEX_BINDINGS);
        return;
    }

    if (buffer == nullptr) {
        log::Warn("BindVertexBuffer called with null buffer");
        return;
    }

    state_.vertexBuffers[binding] = buffer;
    state_.vertexOffsets[binding] = offset;

    if (state_.pipeline != nullptr) {
        state_.pipeline->BindVertexBuffer(binding, buffer, offset);
    }
}

void OpenGLCommandBuffer::BindIndexBuffer(Buffer* buffer, IndexType indexType) {
    if (buffer == nullptr) {
        log::Warn("BindIndexBuffer called with null buffer");
        return;
    }

    state_.indexBuffer = buffer;
    state_.indexType = indexType;

    if (state_.pipeline != nullptr) {
        state_.pipeline->BindIndexBuffer(buffer, indexType);
    }
}

void OpenGLCommandBuffer::Draw(
    u32 vertexCount,
    u32 instanceCount,
    u32 firstVertex,
    u32 firstInstance
) {
    if (!isInsideRenderPass_) {
        log::Error("Draw called outside of render pass");
        return;
    }

    if (state_.pipeline == nullptr) {
        log::Error("Draw called without bound pipeline");
        return;
    }

    state_.pipeline->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void OpenGLCommandBuffer::DrawIndexed(
    u32 indexCount,
    u32 instanceCount,
    u32 firstIndex,
    i32 vertexOffset,
    u32 firstInstance
) {
    if (!isInsideRenderPass_) {
        log::Error("DrawIndexed called outside of render pass");
        return;
    }

    if (state_.pipeline == nullptr) {
        log::Error("DrawIndexed called without bound pipeline");
        return;
    }

    if (state_.indexBuffer == nullptr) {
        log::Error("DrawIndexed called without bound index buffer");
        return;
    }

    state_.pipeline->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void OpenGLCommandBuffer::DrawFullscreenQuad() {
    if (!isInsideRenderPass_) {
        log::Error("DrawFullscreenQuad called outside of render pass");
        return;
    }

    if (state_.pipeline == nullptr) {
        log::Error("DrawFullscreenQuad called without bound pipeline");
        return;
    }

    glBindVertexArray(fullscreenQuadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (state_.pipeline->GetVertexLayout() != nullptr) {
        state_.pipeline->Bind();
    }
}

void OpenGLCommandBuffer::CopyBuffer(
    Buffer* src,
    Buffer* dst,
    u64 size,
    u64 srcOffset,
    u64 dstOffset
) {
    if (src == nullptr || dst == nullptr) {
        log::Error("CopyBuffer called with null buffer");
        return;
    }

    glCopyNamedBufferSubData(
        src->GetHandle(),
        dst->GetHandle(),
        static_cast<GLintptr>(srcOffset),
        static_cast<GLintptr>(dstOffset),
        static_cast<GLsizeiptr>(size)
    );
}

void OpenGLCommandBuffer::PushConstants(
    ShaderStage /*stage*/,
    u32 /*offset*/,
    u32 /*size*/,
    const void* /*data*/
) {
    log::Warn("PushConstants not supported in OpenGL backend, use uniform buffers instead");
}

[[nodiscard]] scope<CommandBuffer> CreateOpenGLCommandBuffer(Device* device) {
    return scope<CommandBuffer>(new OpenGLCommandBuffer(device));
}

} // namespace cc::gfx
