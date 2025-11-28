#include <cc/gfx/shader/shader.hpp>
#include <cc/gfx/shader/compiler.hpp>
#include <cc/gfx/shader/reflection.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_shader.hpp"
#include <stdexcept>
#include <unordered_map>
#include <fstream>

namespace cc::gfx {

namespace {

struct StageReflectionStorage {
    ShaderReflectionData data;
    ShaderReflection view;
};

[[nodiscard]] std::vector<std::pair<ShaderStage, std::filesystem::path>>
BuildFileStageList(const std::vector<Shader::Builder::StageInfo>& stages) {
    std::vector<std::pair<ShaderStage, std::filesystem::path>> fileStages;
    fileStages.reserve(stages.size());

    for (const auto& info : stages) {
        if (info.isFile) {
            fileStages.emplace_back(info.stage, info.filepath);
        }
    }

    return fileStages;
}

[[nodiscard]] std::vector<std::pair<ShaderStage, std::string>>
BuildUnifiedSourceStageList(const std::vector<Shader::Builder::StageInfo>& stages) {
    std::vector<std::pair<ShaderStage, std::string>> unified;
    unified.reserve(stages.size());

    for (const auto& info : stages) {
        if (info.isFile) {
            std::string source = OpenGLShaderImpl::LoadShaderSource(info.filepath);
            unified.emplace_back(info.stage, std::move(source));
        } else {
            unified.emplace_back(info.stage, info.source);
        }
    }

    return unified;
}

[[nodiscard]] std::vector<Shader::StageSource>
BuildStageMetadata(const std::vector<Shader::Builder::StageInfo>& stages) {
    std::vector<Shader::StageSource> out;
    out.reserve(stages.size());

    for (const auto& info : stages) {
        Shader::StageSource s{};
        s.stage = info.stage;
        s.name  = !info.name.empty()
                ? info.name
                : (info.isFile ? info.filepath.filename().string() : std::string("shader"));
        if (!info.isFile) {
            s.source = info.source;
        }
        out.push_back(std::move(s));
    }

    return out;
}

[[nodiscard]] bool AllStagesAreFiles(const std::vector<Shader::Builder::StageInfo>& stages) {
    for (const auto& info : stages) {
        if (!info.isFile) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::unordered_map<ShaderStage, StageReflectionStorage>
BuildReflections(
    const std::vector<Shader::Builder::StageInfo>& stages
) {
    std::unordered_map<ShaderStage, StageReflectionStorage> reflections;

    auto compiler  = ShaderCompiler::Create();
    auto reflector = ShaderReflector::Create();

    for (const auto& info : stages) {
        std::string source;
        std::string name;

        if (info.isFile) {
            std::filesystem::path path = info.filepath;
            if (!std::filesystem::exists(path)) {
                log::Error("Reflection: shader file not found: {}", path.string());
                continue;
            }

            std::ifstream file(path, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                log::Error("Reflection: failed to open shader file: {}", path.string());
                continue;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            source = buffer.str();
            name   = path.filename().string();
        } else {
            source = info.source;
            name   = info.name.empty() ? "shader" : info.name;
        }

        if (source.empty()) {
            log::Warn("Reflection: empty source for stage {}, skipping", static_cast<int>(info.stage));
            continue;
        }

        std::vector<u32> spirv;
        try {
            ShaderCompileOptions opts{};
            opts.optimize        = false;
            opts.debugInfo       = true;
            opts.warningsAsErrors = false;

            spirv = compiler->CompileSource(source, name, info.stage, opts);
        } catch (const std::exception& e) {
            log::Error("Reflection: compile failed for '{}' (stage {}): {}",
                       name, static_cast<int>(info.stage), e.what());
            continue;
        }

        try {
            StageReflectionStorage storage{};
            storage.data = reflector->Reflect(spirv, info.stage);
            storage.view = storage.data.AsView();
            reflections.emplace(info.stage, std::move(storage));
        } catch (const std::exception& e) {
            log::Error("Reflection: SPIR-V reflection failed for '{}' (stage {}): {}",
                       name, static_cast<int>(info.stage), e.what());
        }
    }

    return reflections;
}

[[nodiscard]] std::unordered_map<ShaderStage, ShaderReflection>
ExtractReflectionViews(std::unordered_map<ShaderStage, StageReflectionStorage>& storage) {
    std::unordered_map<ShaderStage, ShaderReflection> views;
    views.reserve(storage.size());

    for (auto& [stage, s] : storage) {
        views.emplace(stage, s.view);
    }

    return views;
}

} // anonymous namespace

[[nodiscard]] Shader::Builder Shader::Create(Device* device) {
    Builder builder;
    builder.device_ = device;
    return builder;
}

Shader::Builder& Shader::Builder::AddStage(ShaderStage stage, const std::filesystem::path& filepath) {
    StageInfo info{};
    info.stage    = stage;
    info.filepath = filepath;
    info.name     = filepath.filename().string();
    info.isFile   = true;

    stages_.push_back(std::move(info));

    log::Info("Added shader stage from file: {}", filepath.string());
    return *this;
}

Shader::Builder& Shader::Builder::AddStageFromSource(
    ShaderStage stage,
    std::string_view source,
    std::string_view name
) {
    StageInfo info{};
    info.stage = stage;
    info.source.assign(source.begin(), source.end());
    info.name.assign(name.begin(), name.end());
    info.isFile = false;

    stages_.push_back(std::move(info));

    log::Info("Added shader stage from source: {}", info.name);
    return *this;
}

Shader::Builder& Shader::Builder::EnableReflection(bool enable) {
    enableReflection_ = enable;
    return *this;
}

Shader::Builder& Shader::Builder::EnableCache(bool enable) {
    enableCache_ = enable;
    return *this;
}

[[nodiscard]] scope<Shader> Shader::Builder::Build() {
    if (stages_.empty()) {
        log::Error("No shader stages added");
        throw std::runtime_error("No shader stages added");
    }

    if (device_ == nullptr) {
        log::Error("Device is required to create shader");
        throw std::runtime_error("Device is null");
    }

    std::unordered_map<ShaderStage, StageReflectionStorage> reflectionStorage;
    if (enableReflection_) {
        reflectionStorage = BuildReflections(stages_);
    }

    auto stageMeta = BuildStageMetadata(stages_);

    switch (device_->GetBackend()) {
        case Backend::OpenGL: {
            const bool allFiles = AllStagesAreFiles(stages_);

            if (allFiles && !enableCache_) {
                auto fileStages = BuildFileStageList(stages_);
                auto shader     = CreateOpenGLShader(device_, fileStages);
                shader->stages_ = std::move(stageMeta);
                if (enableReflection_) {
                    shader->reflections_ = ExtractReflectionViews(reflectionStorage);
                }
                return shader;
            }

            auto unifiedSources = BuildUnifiedSourceStageList(stages_);

            if (enableCache_) {
                log::Info("Shader cache enabled: keeping per-stage source in memory");
            }

            auto shader = CreateOpenGLShaderFromSource(device_, unifiedSources);

            shader->stages_.clear();
            shader->stages_.reserve(unifiedSources.size());
            for (size_t i = 0; i < unifiedSources.size(); ++i) {
                StageSource s{};
                s.stage  = unifiedSources[i].first;
                s.source = unifiedSources[i].second;
                s.name   = (i < stageMeta.size() && !stageMeta[i].name.empty())
                         ? stageMeta[i].name
                         : std::string("shader");
                shader->stages_.push_back(std::move(s));
            }

            if (enableReflection_) {
                shader->reflections_ = ExtractReflectionViews(reflectionStorage);
            }

            return shader;
        }
        case Backend::Vulkan:
            log::Critical("Vulkan shader backend not implemented");
            throw std::runtime_error("Vulkan shader backend not implemented");
        case Backend::Metal:
            log::Critical("Metal shader backend not implemented");
            throw std::runtime_error("Metal shader backend not implemented");
    }

    log::Critical("Unknown backend in Shader::Builder::Build");
    throw std::runtime_error("Unknown backend");
}

Shader::Shader(
    scope<ShaderImpl> impl,
    std::unordered_map<ShaderStage, ShaderReflection> reflections,
    std::vector<StageSource> stages
) noexcept
    : impl_(std::move(impl))
    , reflections_(std::move(reflections))
    , stages_(std::move(stages)) {}

Shader::~Shader() = default;

void Shader::Bind() const {
    impl_->Bind();
}

void Shader::Unbind() const {
    impl_->Unbind();
}

u32 Shader::GetHandle() const noexcept {
    return impl_->GetHandle();
}

const ShaderReflection* Shader::GetReflection(ShaderStage stage) const noexcept {
    const auto it = reflections_.find(stage);
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
