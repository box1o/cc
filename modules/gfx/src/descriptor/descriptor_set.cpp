#include <cc/gfx/descriptor/descriptor_set.hpp>
#include <cc/gfx/descriptor/descriptor_set_layout.hpp>
#include <cc/gfx/buffer/buffer.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_descriptor_set.hpp"
#include <stdexcept>

namespace cc::gfx {

[[nodiscard]] DescriptorSet::Builder DescriptorSet::Create(Device* device, DescriptorSetLayout* layout) {
    Builder builder;
    builder.device_ = device;
    builder.layout_ = layout;
    return builder;
}

DescriptorSet::Builder& DescriptorSet::Builder::Bind(u32 binding, Buffer* buffer, u64 offset, u64 range) {
    if (buffer == nullptr) {
        log::Warn("Attempting to bind null buffer to binding {}", binding);
        return *this;
    }

    BufferBinding bb{};
    bb.binding = binding;
    bb.buffer = buffer;
    bb.offset = offset;
    bb.range = (range == 0) ? buffer->GetSize() : range;
    bufferBindings_.push_back(bb);

    return *this;
}

DescriptorSet::Builder& DescriptorSet::Builder::Bind(u32 binding, Texture* texture, Sampler* sampler) {
    if (texture == nullptr) {
        log::Warn("Attempting to bind null texture to binding {}", binding);
        return *this;
    }

    TextureBinding tb{};
    tb.binding = binding;
    tb.texture = texture;
    tb.sampler = sampler;
    textureBindings_.push_back(tb);

    return *this;
}

[[nodiscard]] scope<DescriptorSet> DescriptorSet::Builder::Build() {
    if (device_ == nullptr) {
        log::Critical("Device is required to create DescriptorSet");
        throw std::runtime_error("Device is null");
    }

    if (layout_ == nullptr) {
        log::Critical("DescriptorSetLayout is required to create DescriptorSet");
        throw std::runtime_error("DescriptorSetLayout is null");
    }

    switch (device_->GetBackend()) {
        case Backend::OpenGL:
            return CreateOpenGLDescriptorSet(device_, layout_, bufferBindings_, textureBindings_);
        case Backend::Vulkan:
            log::Critical("Vulkan descriptor set backend not implemented");
            throw std::runtime_error("Vulkan descriptor set backend not implemented");
        case Backend::Metal:
            log::Critical("Metal descriptor set backend not implemented");
            throw std::runtime_error("Metal descriptor set backend not implemented");
    }

    log::Critical("Unknown backend in DescriptorSet::Builder::Build");
    throw std::runtime_error("Unknown backend");
}

DescriptorSet::DescriptorSet(DescriptorSetLayout* layout) noexcept
: layout_(layout) {}

} // namespace cc::gfx
