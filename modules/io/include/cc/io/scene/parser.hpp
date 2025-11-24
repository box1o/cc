#pragma once
#include "cc/core/core.hpp"
#include "cc/graphics/shader/compiler.hpp"
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cc {

class ShaderParser {
public:
    ShaderParser() = default;

    auto Parse(const std::filesystem::path& filepath) -> result<void>;
    auto ParseString(std::string_view source) -> result<void>;

    [[nodiscard]] const auto& GetShaderSources() const noexcept { return shader_sources_; }
    [[nodiscard]] std::string_view GetShaderName() const noexcept { return shader_name_; }

    [[nodiscard]] bool HasStage(ShaderStage stage) const noexcept {
        return shader_sources_.contains(stage);
    }

    [[nodiscard]] std::string_view GetSource(ShaderStage stage) const noexcept {
        auto it = shader_sources_.find(stage);
        return it != shader_sources_.end() ? it->second : "";
    }

private:
    auto ReadFile(const std::filesystem::path& filepath) -> result<std::string>;
    auto ProcessSource(std::string_view source) -> result<void>;

    std::unordered_map<ShaderStage, std::string> shader_sources_;
    std::string shader_name_{"Unnamed"};

    static constexpr std::string_view TYPE_TOKEN = "#type";
    static constexpr std::string_view NAME_TOKEN = "#name";

    static inline const std::unordered_map<std::string_view, ShaderStage> STAGE_MAP{
        {"vertex", ShaderStage::Vertex},
        {"fragment", ShaderStage::Fragment},
        {"geometry", ShaderStage::Geometry},
        {"compute", ShaderStage::Compute},
        {"tesscontrol", ShaderStage::TessControl},
        {"tessevaluation", ShaderStage::TessEvaluation}
    };
};

}
