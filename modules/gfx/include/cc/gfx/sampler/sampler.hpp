#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

class Sampler {
public:
    ~Sampler();

    static scope<Sampler> Create(Device* device, const SamplerConfig& config = {});

    void Bind(u32 slot) const;
    u32 GetHandle() const;
    const SamplerConfig& GetConfig() const { return config_; }

private:
    Sampler(const SamplerConfig& config, scope<SamplerImpl> impl);

    SamplerConfig config_;
    scope<SamplerImpl> impl_;

    friend scope<Sampler> CreateOpenGLSampler(Device*, const SamplerConfig&);
};

class SamplerImpl {
public:
    virtual ~SamplerImpl() = default;
    virtual void Bind(u32 slot) const = 0;
    virtual u32 GetHandle() const = 0;

protected:
    SamplerImpl() = default;
};

} // namespace cc::gfx
