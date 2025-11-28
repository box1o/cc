#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <vector>
#include <string>

namespace cc::gfx {

struct ShaderReflectionData {
    std::vector<std::string> attributeNames;
    std::vector<ShaderVertexAttribute> attributes;

    std::vector<std::string> uniformBlockNames;
    std::vector<std::string> uniformMemberNames;
    std::vector<UniformBlockMember> uniformMembers;
    std::vector<UniformBlock> uniformBlocks;

    std::vector<std::string> samplerNames;
    std::vector<SamplerBinding> samplers;

    [[nodiscard]] ShaderReflection AsView() const noexcept {
        ShaderReflection view{};
        view.attributes        = attributes.empty()      ? nullptr : attributes.data();
        view.attributeCount    = static_cast<u32>(attributes.size());
        view.uniformBlocks     = uniformBlocks.empty()   ? nullptr : uniformBlocks.data();
        view.uniformBlockCount = static_cast<u32>(uniformBlocks.size());
        view.samplers          = samplers.empty()        ? nullptr : samplers.data();
        view.samplerCount      = static_cast<u32>(samplers.size());
        return view;
    }
};

class ShaderReflector {
public:
    ~ShaderReflector();

    [[nodiscard]] static scope<ShaderReflector> Create();

    [[nodiscard]] ShaderReflectionData Reflect(const std::vector<u32>& spirv, ShaderStage stage);

private:
    ShaderReflector() = default;
};

} // namespace cc::gfx
