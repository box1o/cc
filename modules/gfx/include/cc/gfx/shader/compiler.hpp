#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <filesystem>
#include <string_view>
#include <vector>

namespace cc::gfx {

struct ShaderCompileOptions {
    bool optimize{true};
    bool debugInfo{false};
    bool warningsAsErrors{false};
    const char* entryPoint{"main"};
};

class ShaderCompiler {
public:
    ~ShaderCompiler();

    [[nodiscard]] static scope<ShaderCompiler> Create();

    [[nodiscard]] std::vector<u32> CompileFile(
        const std::filesystem::path& filepath,
        ShaderStage stage,
        const ShaderCompileOptions& options = {}
    );

    [[nodiscard]] std::vector<u32> CompileSource(
        std::string_view source,
        std::string_view name,
        ShaderStage stage,
        const ShaderCompileOptions& options = {}
    );

private:
    ShaderCompiler() = default;
    void Initialize();

    void* compiler_{nullptr};
    void* options_{nullptr};
};

} // namespace cc::gfx
