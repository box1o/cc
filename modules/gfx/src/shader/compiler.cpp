#include <cc/gfx/shader/compiler.hpp>
#include <cc/core/logger.hpp>
#include <shaderc/shaderc.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace cc::gfx {

namespace {

shaderc_shader_kind ToShaderCKind(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex:                  return shaderc_vertex_shader;
        case ShaderStage::Fragment:                return shaderc_fragment_shader;
        case ShaderStage::Geometry:                return shaderc_geometry_shader;
        case ShaderStage::Compute:                 return shaderc_compute_shader;
        case ShaderStage::TessellationControl:     return shaderc_tess_control_shader;
        case ShaderStage::TessellationEvaluation:  return shaderc_tess_evaluation_shader;
    }
    return shaderc_vertex_shader;
}

} // anonymous namespace

ShaderCompiler::~ShaderCompiler() {
    if (options_ != nullptr) {
        delete static_cast<shaderc::CompileOptions*>(options_);
        options_ = nullptr;
    }
    if (compiler_ != nullptr) {
        delete static_cast<shaderc::Compiler*>(compiler_);
        compiler_ = nullptr;
    }
}

scope<ShaderCompiler> ShaderCompiler::Create() {
    auto compiler = scope<ShaderCompiler>(new ShaderCompiler());
    compiler->Initialize();
    return compiler;
}

void ShaderCompiler::Initialize() {
    compiler_ = new shaderc::Compiler();
    options_ = new shaderc::CompileOptions();

    if (! static_cast<shaderc::Compiler*>(compiler_)->IsValid()) {
        throw std::runtime_error("Failed to initialize shaderc compiler");
    }

    log::Info("Shader compiler initialized");
}

std::vector<u32> ShaderCompiler::CompileFile(
    const std::filesystem::path& filepath,
    ShaderStage stage,
    const ShaderCompileOptions& options
) {
    if (!std::filesystem::exists(filepath)) {
        log::Error("Shader file not found: {}", filepath.string());
        throw std::runtime_error("Shader file not found");
    }

    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        log::Error("Failed to open shader file: {}", filepath.string());
        throw std::runtime_error("Failed to open shader file");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    return CompileSource(source, filepath. filename().string(), stage, options);
}

std::vector<u32> ShaderCompiler::CompileSource(
    std::string_view source,
    std::string_view name,
    ShaderStage stage,
    const ShaderCompileOptions& options
) {
    auto* compiler = static_cast<shaderc::Compiler*>(compiler_);
    auto* compileOpts = static_cast<shaderc::CompileOptions*>(options_);

    compileOpts->SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
    compileOpts->SetTargetSpirv(shaderc_spirv_version_1_0);

    if (options.optimize) {
        compileOpts->SetOptimizationLevel(shaderc_optimization_level_performance);
    }

    if (options.debugInfo) {
        compileOpts->SetGenerateDebugInfo();
    }

    if (options.warningsAsErrors) {
        compileOpts->SetWarningsAsErrors();
    }

    shaderc::SpvCompilationResult result = compiler->CompileGlslToSpv(
        source.data(),
        source.size(),
        ToShaderCKind(stage),
        name.data(),
        options.entryPoint,
        *compileOpts
    );

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        log::Error("Shader compilation failed: {}", result. GetErrorMessage());
        throw std::runtime_error("Shader compilation failed");
    }

    std::vector<u32> spirv(result.cbegin(), result.cend());

    log::Info("Compiled shader '{}' to SPIR-V ({} bytes)", name, spirv.size() * 4);

    return spirv;
}

} // namespace cc::gfx
