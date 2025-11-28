#include "gl_descriptor_set_layout.hpp"
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>

namespace cc::gfx {

OpenGLDescriptorSetLayout::OpenGLDescriptorSetLayout(std::vector<DescriptorBinding> bindings)
    : DescriptorSetLayout(std::move(bindings))
    , handle_(s_nextHandle++) {

    log::Trace("OpenGL DescriptorSetLayout created (handle={}, {} bindings)", handle_, bindings_.size());

    for (const auto& binding : bindings_) {
        const char* typeStr = "Unknown";
        switch (binding.type) {
            case DescriptorType::UniformBuffer:        typeStr = "UniformBuffer"; break;
            case DescriptorType::StorageBuffer:        typeStr = "StorageBuffer"; break;
            case DescriptorType::SampledTexture:       typeStr = "SampledTexture"; break;
            case DescriptorType::StorageTexture:       typeStr = "StorageTexture"; break;
            case DescriptorType::Sampler:              typeStr = "Sampler"; break;
            case DescriptorType::CombinedImageSampler: typeStr = "CombinedImageSampler"; break;
        }
        log::Trace("  Binding {}: {} (count={})", binding.binding, typeStr, binding.count);
    }
}

OpenGLDescriptorSetLayout::~OpenGLDescriptorSetLayout() {
    log::Trace("OpenGL DescriptorSetLayout destroyed (handle={})", handle_);
}

[[nodiscard]] scope<DescriptorSetLayout> CreateOpenGLDescriptorSetLayout(
    Device* /*device*/,
    const std::vector<DescriptorBinding>& bindings
) {
    return scope<DescriptorSetLayout>(new OpenGLDescriptorSetLayout(bindings));
}

} // namespace cc::gfx
