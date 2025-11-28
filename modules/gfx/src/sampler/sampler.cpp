#include <cc/gfx/sampler/sampler.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_sampler.hpp"
#include <stdexcept>

namespace cc::gfx {

[[nodiscard]] scope<Sampler> Sampler::Create(Device* device, const SamplerConfig& config) {
    if (device == nullptr) {
        log::Critical("Device is required to create Sampler");
        throw std::runtime_error("Device is null");
    }

    switch (device->GetBackend()) {
        case Backend::OpenGL:
            return CreateOpenGLSampler(device, config);
        case Backend::Vulkan:
            log::Critical("Vulkan Sampler backend not implemented");
            throw std::runtime_error("Vulkan Sampler backend not implemented");
        case Backend::Metal:
            log::Critical("Metal Sampler backend not implemented");
            throw std::runtime_error("Metal Sampler backend not implemented");
    }

    log::Critical("Unknown backend for Sampler::Create");
    throw std::runtime_error("Unknown backend");
}

Sampler::Sampler(const SamplerConfig& config, scope<SamplerImpl> impl) noexcept
    : config_(config)
    , impl_(std::move(impl)) {}

Sampler::~Sampler() = default;

void Sampler::Bind(u32 slot) const {
    impl_->Bind(slot);
}

u32 Sampler::GetHandle() const noexcept {
    return impl_->GetHandle();
}

} // namespace cc::gfx
