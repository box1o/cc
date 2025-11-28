#include "gl_shader.hpp"
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace cc::gfx {

std::string OpenGLShaderImpl::LoadShaderSource(const std::filesystem::path& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        log::Error("Failed to open shader file: {}", filepath.string());
        throw std::runtime_error("Failed to open shader file");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int OpenGLShaderImpl::GetGLShaderStage(ShaderStage stage) noexcept {
    switch (stage) {
        case ShaderStage::Vertex:                 return GL_VERTEX_SHADER;
        case ShaderStage::Fragment:               return GL_FRAGMENT_SHADER;
        case ShaderStage::Geometry:               return GL_GEOMETRY_SHADER;
        case ShaderStage::Compute:                return GL_COMPUTE_SHADER;
        case ShaderStage::TessellationControl:    return GL_TESS_CONTROL_SHADER;
        case ShaderStage::TessellationEvaluation: return GL_TESS_EVALUATION_SHADER;
    }
    return GL_VERTEX_SHADER;
}

const char* OpenGLShaderImpl::GetShaderStageName(ShaderStage stage) noexcept {
    switch (stage) {
        case ShaderStage::Vertex:                 return "Vertex";
        case ShaderStage::Fragment:               return "Fragment";
        case ShaderStage::Geometry:               return "Geometry";
        case ShaderStage::Compute:                return "Compute";
        case ShaderStage::TessellationControl:    return "TessControl";
        case ShaderStage::TessellationEvaluation: return "TessEvaluation";
    }
    return "Unknown";
}

u32 OpenGLShaderImpl::CompileShader(const std::string& source, ShaderStage stage) {
    const unsigned int glStage = GetGLShaderStage(stage);
    const u32 shader = glCreateShader(glStage);

    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == 0) {
        int logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::string infoLog(static_cast<size_t>(logLength), '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());

        glDeleteShader(shader);

        log::Error("{} shader compilation failed:\n{}", GetShaderStageName(stage), infoLog);
        throw std::runtime_error("Shader compilation failed");
    }

    log::Trace("{} shader compiled successfully (handle={})", GetShaderStageName(stage), shader);
    return shader;
}

u32 OpenGLShaderImpl::LinkProgram(const std::vector<u32>& shaderIDs) {
    const u32 program = glCreateProgram();

    for (const u32 shader : shaderIDs) {
        glAttachShader(program, shader);
    }

    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == 0) {
        int logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::string infoLog(static_cast<size_t>(logLength), '\0');
        glGetProgramInfoLog(program, logLength, nullptr, infoLog.data());

        glDeleteProgram(program);

        log::Error("Shader program linking failed:\n{}", infoLog);
        throw std::runtime_error("Shader program linking failed");
    }

    for (const u32 shader : shaderIDs) {
        glDetachShader(program, shader);
    }

    log::Info("Shader program linked successfully (handle={})", program);
    return program;
}

void OpenGLShaderImpl::DeleteShader(u32 shader) {
    if (shader != 0) {
        glDeleteShader(shader);
    }
}

void OpenGLShaderImpl::DeleteProgram(u32 program) {
    if (program != 0) {
        glDeleteProgram(program);
    }
}

OpenGLShaderImpl::OpenGLShaderImpl(const std::vector<std::pair<ShaderStage, std::string>>& stages) {
    std::vector<u32> shaderIDs;
    shaderIDs.reserve(stages.size());

    try {
        for (const auto& [stage, source] : stages) {
            const u32 shaderID = CompileShader(source, stage);
            shaderIDs.push_back(shaderID);
        }

        program_ = LinkProgram(shaderIDs);

        for (const u32 shader : shaderIDs) {
            DeleteShader(shader);
        }

    } catch (...) {
        for (const u32 shader : shaderIDs) {
            DeleteShader(shader);
        }
        throw;
    }
}

OpenGLShaderImpl::~OpenGLShaderImpl() {
    if (program_ != 0) {
        DeleteProgram(program_);
        log::Trace("OpenGL shader program destroyed (handle={})", program_);
        program_ = 0;
    }
}

void OpenGLShaderImpl::Bind() const {
    glUseProgram(program_);
}

void OpenGLShaderImpl::Unbind() const {
    glUseProgram(0);
}

void OpenGLShaderImpl::SetUniformBlockBinding(const char* name, u32 binding) const {
    const unsigned int blockIndex = glGetUniformBlockIndex(program_, name);
    if (blockIndex == GL_INVALID_INDEX) {
        log::Warn("Uniform block '{}' not found in shader program", name);
        return;
    }

    glUniformBlockBinding(program_, blockIndex, binding);
    log::Trace("Set uniform block '{}' to binding {}", name, binding);
}

void OpenGLShaderImpl::SetSamplerBinding(const char* name, u32 binding) const {
    const int location = glGetUniformLocation(program_, name);
    if (location == -1) {
        log::Warn("Sampler '{}' not found in shader program", name);
        return;
    }

    glProgramUniform1i(program_, location, static_cast<int>(binding));
    log::Trace("Set sampler '{}' to binding {}", name, binding);
}

[[nodiscard]] scope<Shader> CreateOpenGLShader(
    Device* /*device*/,
    const std::vector<std::pair<ShaderStage, std::filesystem::path>>& stages
) {
    std::vector<std::pair<ShaderStage, std::string>> stageSources;
    stageSources.reserve(stages.size());

    for (const auto& [stage, filepath] : stages) {
        std::string source = OpenGLShaderImpl::LoadShaderSource(filepath);
        stageSources.emplace_back(stage, std::move(source));
    }

    auto impl = scope<ShaderImpl>(new OpenGLShaderImpl(stageSources));
    return scope<Shader>(new Shader(std::move(impl), {}));
}

[[nodiscard]] scope<Shader> CreateOpenGLShaderFromSource(
    Device* /*device*/,
    const std::vector<std::pair<ShaderStage, std::string>>& stages
) {
    auto impl = scope<ShaderImpl>(new OpenGLShaderImpl(stages));
    return scope<Shader>(new Shader(std::move(impl), {}));
}

} // namespace cc::gfx
