#include <cc/gfx/descriptor/descriptor_set_layout.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>

namespace cc::gfx {

DescriptorSetLayout::Builder DescriptorSetLayout::Create(Device* device) {
    Builder builder;
    builder.device_ = device;
    return builder;
}

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::Binding(u32 binding, DescriptorType type, ShaderStage stages, u32 count) {
    DescriptorBinding db{};
    db.binding = binding;
    db.type = type;
    db. stages = stages;
    db.count = count;
    bindings_.push_back(db);
    return *this;
}

scope<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() {
    if (device_ == nullptr) {
        log::Critical("Device is required to create DescriptorSetLayout");
        throw std::runtime_error("Device is null");
    }

    if (bindings_.empty()) {
        log::Error("DescriptorSetLayout requires at least one binding");
        throw std::runtime_error("DescriptorSetLayout requires at least one binding");
    }

    return device_->CreateDescriptorSetLayout(bindings_);
}

DescriptorSetLayout::DescriptorSetLayout(std::vector<DescriptorBinding> bindings)
    : bindings_(std::move(bindings)) {
    log::Trace("DescriptorSetLayout created ({} bindings)", bindings_.size());
}

const DescriptorBinding* DescriptorSetLayout::GetBinding(u32 binding) const {
    for (const auto& b : bindings_) {
        if (b.binding == binding) {
            return &b;
        }
    }
    return nullptr;
}

bool DescriptorSetLayout::HasBinding(u32 binding) const {
    return GetBinding(binding) != nullptr;
}

} // namespace cc::gfx
