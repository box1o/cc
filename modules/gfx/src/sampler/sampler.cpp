#include <cc/gfx/sampler/sampler.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>

namespace cc::gfx {

scope<Sampler> Sampler::Create(Device* device, const SamplerConfig& config) {
    if (device == nullptr) {
        log::Critical("Device is required to create Sampler");
        throw std::runtime_error("Device is null");
    }

    return device->CreateSampler(config);
}

Sampler::Sampler(const SamplerConfig& config, scope<SamplerImpl> impl)
    : config_(config), impl_(std::move(impl)) {}

Sampler::~Sampler() = default;

void Sampler::Bind(u32 slot) const {
    impl_->Bind(slot);
}

u32 Sampler::GetHandle() const {
    return impl_->GetHandle();
}

} // namespace cc::gfx
