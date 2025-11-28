#pragma once
#include <cc/gfx/shader/shader.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>
#include <vector>
#include <string>
#include <filesystem>

namespace cc::gfx {

class Device;

class OpenGLShaderImpl final : public ShaderImpl {
public:
    explicit OpenGLShaderImpl(const std::vector<std::pair<ShaderStage, std::string>>& stages);
    ~OpenGLShaderImpl() override;

    void Bind() const override;
    void Unbind() const override;
    [[nodiscard]] u32 GetHandle() const noexcept override { return program_; }

    void SetUniformBlockBinding(const char* name, u32 binding) const override;
    void SetSamplerBinding(const char* name, u32 binding) const override;

    [[nodiscard]] static std::string LoadShaderSource(const std::filesystem::path& filepath);

private:
    [[nodiscard]] static unsigned int GetGLShaderStage(ShaderStage stage) noexcept;
    [[nodiscard]] static const char* GetShaderStageName(ShaderStage stage) noexcept;
    [[nodiscard]] static u32 CompileShader(const std::string& source, ShaderStage stage);
    [[nodiscard]] static u32 LinkProgram(const std::vector<u32>& shaderIDs);
    static void DeleteShader(u32 shader);
    static void DeleteProgram(u32 program);

    u32 program_{0};
};

[[nodiscard]] scope<Shader> CreateOpenGLShader(
    Device* device,
    const std::vector<std::pair<ShaderStage, std::filesystem::path>>& stages
);

[[nodiscard]] scope<Shader> CreateOpenGLShaderFromSource(
    Device* device,
    const std::vector<std::pair<ShaderStage, std::string>>& stages
);

} // namespace cc::gfx
