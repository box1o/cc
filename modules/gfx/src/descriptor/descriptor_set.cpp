#include <cc/gfx/descriptor/descriptor_set.hpp>
#include <cc/gfx/descriptor/descriptor_set_layout.hpp>
#include <cc/gfx/buffer/buffer.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>

namespace cc::gfx {

DescriptorSet::Builder DescriptorSet::Create(Device* device, ref<DescriptorSetLayout> layout) {
    Builder builder;
    builder.device_ = device;
    builder.layout_ = layout;
    return builder;
}

DescriptorSet::Builder& DescriptorSet::Builder::Bind(u32 binding, ref<Buffer> buffer, u64 offset, u64 range) {
    if (buffer == nullptr) {
        log::Warn("Attempting to bind null buffer to binding {}", binding);
        return *this;
    }

    BufferBinding bb{};
    bb.binding = binding;
    bb.buffer = buffer;
    bb. offset = offset;
    bb.range = (range == 0) ? buffer->GetSize() : range;
    bufferBindings_.push_back(bb);

    return *this;
}

DescriptorSet::Builder& DescriptorSet::Builder::Bind(u32 binding, ref<Texture> texture, ref<Sampler> sampler) {
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

scope<DescriptorSet> DescriptorSet::Builder::Build() {
    if (device_ == nullptr) {
        log::Critical("Device is required to create DescriptorSet");
        throw std::runtime_error("Device is null");
    }

    if (layout_ == nullptr) {
        log::Critical("DescriptorSetLayout is required to create DescriptorSet");
        throw std::runtime_error("DescriptorSetLayout is null");
    }

    return device_->CreateDescriptorSet(layout_, bufferBindings_, textureBindings_);
}

DescriptorSet::DescriptorSet(ref<DescriptorSetLayout> layout)
    : layout_(layout) {}

} // namespace cc::gfx
