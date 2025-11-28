#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

class Sampler {
public:
    ~Sampler();

    [[nodiscard]] static scope<Sampler> Create(Device* device, const SamplerConfig& config = {});

    void Bind(u32 slot) const;
    [[nodiscard]] u32 GetHandle() const noexcept;
    [[nodiscard]] const SamplerConfig& GetConfig() const noexcept { return config_; }

private:
    Sampler(const SamplerConfig& config, scope<SamplerImpl> impl) noexcept;

    SamplerConfig config_{};
    scope<SamplerImpl> impl_;

    friend scope<Sampler> CreateOpenGLSampler(Device*, const SamplerConfig&);
};

class SamplerImpl {
public:
    virtual ~SamplerImpl() = default;
    virtual void Bind(u32 slot) const = 0;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

protected:
    SamplerImpl() = default;
};

} // namespace cc::gfx
