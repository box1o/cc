#include <cc/gfx/shader/shader.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_shader.hpp"
#include <stdexcept>

namespace cc::gfx {

Shader::Builder Shader::Create(Device* device) {
    Builder builder;
    builder.device_ = device;
    return builder;
}

Shader::Builder& Shader::Builder::AddStage(ShaderStage stage, const std::filesystem::path& filepath) {
    StageInfo info;
    info. stage = stage;
    info. filepath = filepath;
    info. isFile = true;

    stages_.push_back(std::move(info));

    log::Info("Added shader stage from file: {}", filepath.string());

    return *this;
}

Shader::Builder& Shader::Builder::AddStageFromSource(ShaderStage stage, std::string_view source, std::string_view name) {
    StageInfo info;
    info.stage = stage;
    info.source = source;
    info.name = name;
    info.isFile = false;

    stages_.push_back(std::move(info));

    log::Info("Added shader stage from source: {}", name);

    return *this;
}

Shader::Builder& Shader::Builder::EnableReflection(bool enable) {
    enableReflection_ = enable;
    return *this;
}

scope<Shader> Shader::Builder::Build() {
    if (stages_.empty()) {
        log::Error("No shader stages added");
        throw std::runtime_error("No shader stages added");
    }

    if (device_ == nullptr) {
        log::Error("Device is required to create shader");
        throw std::runtime_error("Device is null");
    }

    std::vector<std::pair<ShaderStage, std::filesystem::path>> filePaths;
    for (const auto& info : stages_) {
        if (info.isFile) {
            filePaths.emplace_back(info.stage, info.filepath);
        }
    }

    return CreateOpenGLShader(device_, filePaths);
}

Shader::Shader(scope<ShaderImpl> impl, std::unordered_map<ShaderStage, ShaderReflection> reflections)
    : impl_(std::move(impl))
    , reflections_(std::move(reflections)) {}

Shader::~Shader() = default;

void Shader::Bind() const {
    impl_->Bind();
}

void Shader::Unbind() const {
    impl_->Unbind();
}

u32 Shader::GetHandle() const {
    return impl_->GetHandle();
}

const ShaderReflection* Shader::GetReflection(ShaderStage stage) const {
    auto it = reflections_. find(stage);
    if (it == reflections_.end()) {
        return nullptr;
    }
    return &it->second;
}

void Shader::SetUniformBlock(const char* name, u32 binding) const {
    impl_->SetUniformBlockBinding(name, binding);
}

void Shader::SetSampler(const char* name, u32 binding) const {
    impl_->SetSamplerBinding(name, binding);
}

} // namespace cc::gfx
