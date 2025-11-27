#pragma once
#include <cc/gfx/shader/shader.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>
#include <vector>
#include <string>

namespace cc::gfx {

class Device;

class OpenGLShaderImpl final : public ShaderImpl {
public:
    explicit OpenGLShaderImpl(const std::vector<std::pair<ShaderStage, std::string>>& stages);
    ~OpenGLShaderImpl() override;

    void Bind() const override;
    void Unbind() const override;
    u32 GetHandle() const override { return program_; }

    void SetUniformBlockBinding(const char* name, u32 binding) const override;
    void SetSamplerBinding(const char* name, u32 binding) const override;

    //NOTE: Made public for factory function access
    static std::string LoadShaderSource(const std::filesystem::path& filepath);

private:
    static unsigned int GetGLShaderStage(ShaderStage stage);
    static const char* GetShaderStageName(ShaderStage stage);
    static u32 CompileShader(const std::string& source, ShaderStage stage);
    static u32 LinkProgram(const std::vector<u32>& shaderIDs);
    static void DeleteShader(u32 shader);
    static void DeleteProgram(u32 program);

    u32 program_{0};
};

scope<Shader> CreateOpenGLShader(Device* device, const std::vector<std::pair<ShaderStage, std::filesystem::path>>& stages);
scope<Shader> CreateOpenGLShaderFromSource(Device* device, const std::vector<std::pair<ShaderStage, std::string>>& stages);

} // namespace cc::gfx
