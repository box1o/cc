#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <vector>

namespace cc::gfx {

class ShaderReflector {
public:
    ~ShaderReflector();

    [[nodiscard]] static scope<ShaderReflector> Create();

    [[nodiscard]] ShaderReflection Reflect(const std::vector<u32>& spirv, ShaderStage stage);

private:
    ShaderReflector() = default;
};

} // namespace cc::gfx
