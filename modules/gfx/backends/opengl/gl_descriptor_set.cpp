#include "gl_descriptor_set.hpp"
#include <cc/gfx/descriptor/descriptor_set_layout.hpp>
#include <cc/gfx/buffer/buffer.hpp>
#include <cc/gfx/texture/texture.hpp>
#include <cc/gfx/sampler/sampler.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>

namespace cc::gfx {

OpenGLDescriptorSet::OpenGLDescriptorSet(
    DescriptorSetLayout* layout,
    std::vector<BufferBinding> bufferBindings,
    std::vector<TextureBinding> textureBindings
)
    : DescriptorSet(layout)
    , handle_(s_nextHandle++)
    , bufferBindings_(std::move(bufferBindings))
    , textureBindings_(std::move(textureBindings)) {

    log::Trace(
        "OpenGL DescriptorSet created (handle={}, {} buffers, {} textures)",
        handle_,
        bufferBindings_.size(),
        textureBindings_.size()
    );
}

OpenGLDescriptorSet::~OpenGLDescriptorSet() {
    log::Trace("OpenGL DescriptorSet destroyed (handle={})", handle_);
}

void OpenGLDescriptorSet::Bind(u32 setIndex) const {
    BindBuffers(setIndex);
    BindTextures(setIndex);
}

void OpenGLDescriptorSet::BindBuffers(u32 /*setIndex*/) const {
    for (const auto& binding : bufferBindings_) {
        if (binding.buffer == nullptr) {
            continue;
        }

        const DescriptorBinding* layoutBinding = layout_->GetBinding(binding.binding);
        if (layoutBinding == nullptr) {
            log::Warn("Buffer binding {} not found in layout", binding.binding);
            continue;
        }

        const u32 bufferHandle = binding.buffer->GetHandle();

        switch (layoutBinding->type) {
            case DescriptorType::UniformBuffer:
                glBindBufferRange(
                    GL_UNIFORM_BUFFER,
                    binding.binding,
                    bufferHandle,
                    static_cast<GLintptr>(binding.offset),
                    static_cast<GLsizeiptr>(binding.range)
                );
                break;

            case DescriptorType::StorageBuffer:
                glBindBufferRange(
                    GL_SHADER_STORAGE_BUFFER,
                    binding.binding,
                    bufferHandle,
                    static_cast<GLintptr>(binding.offset),
                    static_cast<GLsizeiptr>(binding.range)
                );
                break;

            default:
                log::Warn("Unsupported buffer descriptor type for binding {}", binding.binding);
                break;
        }
    }
}

void OpenGLDescriptorSet::BindTextures(u32 /*setIndex*/) const {
    for (const auto& binding : textureBindings_) {
        if (binding.texture == nullptr) {
            continue;
        }

        const DescriptorBinding* layoutBinding = layout_->GetBinding(binding.binding);
        if (layoutBinding == nullptr) {
            log::Warn("Texture binding {} not found in layout", binding.binding);
            continue;
        }

        binding.texture->Bind(binding.binding);

        if (binding.sampler != nullptr) {
            binding.sampler->Bind(binding.binding);
        }
    }
}

void OpenGLDescriptorSet::Update(u32 binding, Buffer* buffer, u64 offset, u64 range) {
    for (auto& bb : bufferBindings_) {
        if (bb.binding == binding) {
            bb.buffer = buffer;
            bb.offset = offset;
            bb.range = (range == 0 && buffer != nullptr) ? buffer->GetSize() : range;
            log::Trace("DescriptorSet updated buffer at binding {}", binding);
            return;
        }
    }

    BufferBinding newBinding{};
    newBinding.binding = binding;
    newBinding.buffer = buffer;
    newBinding.offset = offset;
    newBinding.range = (range == 0 && buffer != nullptr) ? buffer->GetSize() : range;
    bufferBindings_.push_back(newBinding);
    log::Trace("DescriptorSet added buffer at binding {}", binding);
}

void OpenGLDescriptorSet::Update(u32 binding, Texture* texture, Sampler* sampler) {
    for (auto& tb : textureBindings_) {
        if (tb.binding == binding) {
            tb.texture = texture;
            tb.sampler = sampler;
            log::Trace("DescriptorSet updated texture at binding {}", binding);
            return;
        }
    }

    TextureBinding newBinding{};
    newBinding.binding = binding;
    newBinding.texture = texture;
    newBinding.sampler = sampler;
    textureBindings_.push_back(newBinding);
    log::Trace("DescriptorSet added texture at binding {}", binding);
}

[[nodiscard]] scope<DescriptorSet> CreateOpenGLDescriptorSet(
    Device* /*device*/,
    DescriptorSetLayout* layout,
    const std::vector<BufferBinding>& bufferBindings,
    const std::vector<TextureBinding>& textureBindings
) {
    return scope<DescriptorSet>(new OpenGLDescriptorSet(layout, bufferBindings, textureBindings));
}

} // namespace cc::gfx
