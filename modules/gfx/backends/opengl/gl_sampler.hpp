#pragma once
#include <cc/gfx/sampler/sampler.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>

namespace cc::gfx {

class Device;

class OpenGLSamplerImpl final : public SamplerImpl {
public:
    explicit OpenGLSamplerImpl(const SamplerConfig& config);
    ~OpenGLSamplerImpl() override;

    void Bind(u32 slot) const override;
    u32 GetHandle() const override { return handle_; }

private:
    u32 handle_{0};
};

scope<Sampler> CreateOpenGLSampler(Device* device, const SamplerConfig& config);

} // namespace cc::gfx
