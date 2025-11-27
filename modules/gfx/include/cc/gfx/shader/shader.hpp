#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <filesystem>
#include <vector>
#include <unordered_map>

namespace cc::gfx {

class Shader {
public:
    class Builder {
    public:
        Builder& AddStage(ShaderStage stage, const std::filesystem::path& filepath);
        Builder& AddStageFromSource(ShaderStage stage, std::string_view source, std::string_view name = "shader");
        Builder& EnableReflection(bool enable = true);
        scope<Shader> Build();

    private:
        struct StageInfo {
            ShaderStage stage;
            std::filesystem::path filepath;
            std::string source;
            std::string name;
            bool isFile{true};
        };

        Device* device_{nullptr};
        std::vector<StageInfo> stages_;
        bool enableReflection_{false};

        friend class Shader;
    };

    ~Shader();

    static Builder Create(Device* device);

    void Bind() const;
    void Unbind() const;

    u32 GetHandle() const;
    const ShaderReflection* GetReflection(ShaderStage stage) const;

    void SetUniformBlock(const char* name, u32 binding) const;
    void SetSampler(const char* name, u32 binding) const;

private:
    Shader(scope<ShaderImpl> impl, std::unordered_map<ShaderStage, ShaderReflection> reflections);

    scope<ShaderImpl> impl_;
    std::unordered_map<ShaderStage, ShaderReflection> reflections_;

    friend scope<Shader> CreateOpenGLShader(Device*, const std::vector<std::pair<ShaderStage, std::filesystem::path>>&);
    friend scope<Shader> CreateOpenGLShaderFromSource(Device*, const std::vector<std::pair<ShaderStage, std::string>>&);
};

class ShaderImpl {
public:
    virtual ~ShaderImpl() = default;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual u32 GetHandle() const = 0;
    virtual void SetUniformBlockBinding(const char* name, u32 binding) const = 0;
    virtual void SetSamplerBinding(const char* name, u32 binding) const = 0;

protected:
    ShaderImpl() = default;
};

} // namespace cc::gfx
