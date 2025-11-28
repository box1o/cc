#include <cc/gfx/pipeline/pipeline_validation.hpp>
#include <cc/gfx/pipeline/vertex_layout.hpp>
#include <cc/gfx/shader/shader.hpp>
#include <cc/gfx/descriptor/descriptor_set_layout.hpp>
#include <cc/core/logger.hpp>

namespace cc::gfx {

namespace {

#if CC_GFX_ENABLE_VALIDATION

[[nodiscard]] static bool UniformTypeMatchesVertexFormat(UniformType type, VertexFormat fmt) {
    switch (type) {
        case UniformType::Float:
            return fmt == VertexFormat::Float;
        case UniformType::Vec2:
            return fmt == VertexFormat::Float2;
        case UniformType::Vec3:
            return fmt == VertexFormat::Float3;
        case UniformType::Vec4:
            return fmt == VertexFormat::Float4;
        case UniformType::Int:
            return fmt == VertexFormat::Int || fmt == VertexFormat::UInt;
        case UniformType::IVec2:
            return fmt == VertexFormat::Int2 || fmt == VertexFormat::UInt2;
        case UniformType::IVec3:
            return fmt == VertexFormat::Int3 || fmt == VertexFormat::UInt3;
        case UniformType::IVec4:
            return fmt == VertexFormat::Int4 || fmt == VertexFormat::UInt4;
        default:
            return false;
    }
}

#endif // CC_GFX_ENABLE_VALIDATION

} // anonymous namespace

void ValidateVertexLayoutAgainstShader(const VertexLayout* layout, const Shader* shader) {
#if !CC_GFX_ENABLE_VALIDATION
    (void)layout;
    (void)shader;
    return;
#else
    if (layout == nullptr || shader == nullptr) {
        return;
    }

    const ShaderReflection* refl = shader->GetReflection(ShaderStage::Vertex);
    if (refl == nullptr || refl->attributes == nullptr) {
        log::Trace("Vertex layout validation skipped: no vertex shader reflection data");
        return;
    }

    const auto& attributes = layout->GetAttributes();

    //NOTE: Check each shader attribute has a matching vertex attribute
    for (u32 i = 0; i < refl->attributeCount; ++i) {
        const ShaderVertexAttribute& sa = refl->attributes[i];
        const char* name = sa.name != nullptr ? sa.name : "<unnamed>";
        bool found = false;

        for (const auto& va : attributes) {
            if (va.location == sa.location) {
                found = true;

                if (!UniformTypeMatchesVertexFormat(sa.type, va.format)) {
                    log::Warn(
                        "Vertex layout mismatch at location {} ('{}'): shader type {} does not match vertex format {}",
                        sa.location,
                        name,
                        static_cast<int>(sa.type),
                        static_cast<int>(va.format)
                    );
                }

                const u32 expectedSize = sa.size;
                const u32 actualSize   = VertexLayout::GetFormatSize(va.format);
                if (expectedSize != 0 && actualSize != 0 && expectedSize != actualSize) {
                    log::Warn(
                        "Vertex layout size mismatch at location {} ('{}'): shader size {} bytes, vertex size {} bytes",
                        sa.location,
                        name,
                        expectedSize,
                        actualSize
                    );
                }

                break;
            }
        }

        if (!found) {
            log::Warn(
                "Vertex layout missing attribute for shader input location {} ('{}')",
                sa.location,
                name
            );
        }
    }

    //NOTE: Warn about vertex attributes that are not used by the shader
    for (const auto& va : attributes) {
        bool used = false;
        for (u32 i = 0; i < refl->attributeCount; ++i) {
            if (refl->attributes[i].location == va.location) {
                used = true;
                break;
            }
        }

        if (!used) {
            log::Warn(
                "Vertex attribute at location {} is not consumed by the vertex shader",
                va.location
            );
        }
    }
#endif
}

void ValidateDescriptorSetLayoutAgainstShader(const DescriptorSetLayout* layout, const Shader* shader) {
#if !CC_GFX_ENABLE_VALIDATION
    (void)layout;
    (void)shader;
    return;
#else
    if (layout == nullptr || shader == nullptr) {
        return;
    }

    const ShaderReflection* vsRefl = shader->GetReflection(ShaderStage::Vertex);
    const ShaderReflection* fsRefl = shader->GetReflection(ShaderStage::Fragment);

    if ((vsRefl == nullptr || (vsRefl->uniformBlocks == nullptr && vsRefl->samplers == nullptr)) &&
        (fsRefl == nullptr || (fsRefl->uniformBlocks == nullptr && fsRefl->samplers == nullptr))) {
        log::Trace("Descriptor set layout validation skipped: no reflection data for shader");
        return;
    }

    const auto& bindings = layout->GetBindings();

    auto checkUniformBlock = [&](const UniformBlock& block, const char* stageName) {
        const u32 binding = block.binding;
        const DescriptorBinding* db = layout->GetBinding(binding);
        if (db == nullptr) {
            log::Warn(
                "Uniform block '{}' (binding={}, stage={}) has no matching DescriptorBinding",
                block.name != nullptr ? block.name : "<unnamed>",
                binding,
                stageName
            );
            return;
        }

        if (db->type != DescriptorType::UniformBuffer) {
            log::Warn(
                "Uniform block '{}' (binding={}, stage={}) expects UniformBuffer, but layout binding type is {}",
                block.name != nullptr ? block.name : "<unnamed>",
                binding,
                stageName,
                static_cast<int>(db->type)
            );
        }
    };

    auto checkSampler = [&](const SamplerBinding& sampler, const char* stageName) {
        const u32 binding = sampler.binding;
        const DescriptorBinding* db = layout->GetBinding(binding);
        if (db == nullptr) {
            log::Warn(
                "Sampler '{}' (binding={}, stage={}) has no matching DescriptorBinding",
                sampler.name != nullptr ? sampler.name : "<unnamed>",
                binding,
                stageName
            );
            return;
        }

        const bool isSampled =
            db->type == DescriptorType::SampledTexture ||
            db->type == DescriptorType::CombinedImageSampler;
        if (!isSampled) {
            log::Warn(
                "Sampler '{}' (binding={}, stage={}) expects SampledTexture/CombinedImageSampler, but layout binding type is {}",
                sampler.name != nullptr ? sampler.name : "<unnamed>",
                binding,
                stageName,
                static_cast<int>(db->type)
            );
        }
    };

    if (vsRefl != nullptr) {
        for (u32 i = 0; i < vsRefl->uniformBlockCount; ++i) {
            checkUniformBlock(vsRefl->uniformBlocks[i], "VS");
        }
        for (u32 i = 0; i < vsRefl->samplerCount; ++i) {
            checkSampler(vsRefl->samplers[i], "VS");
        }
    }

    if (fsRefl != nullptr) {
        for (u32 i = 0; i < fsRefl->uniformBlockCount; ++i) {
            checkUniformBlock(fsRefl->uniformBlocks[i], "FS");
        }
        for (u32 i = 0; i < fsRefl->samplerCount; ++i) {
            checkSampler(fsRefl->samplers[i], "FS");
        }
    }

    //NOTE: Warn about descriptor bindings that are never referenced in any shader stage
    for (const auto& db : bindings) {
        bool used = false;

        auto usedIn = [&](const ShaderReflection* refl) {
            if (refl == nullptr) return false;

            for (u32 i = 0; i < refl->uniformBlockCount; ++i) {
                if (refl->uniformBlocks[i].binding == db.binding) {
                    return true;
                }
            }
            for (u32 i = 0; i < refl->samplerCount; ++i) {
                if (refl->samplers[i].binding == db.binding) {
                    return true;
                }
            }
            return false;
        };

        if (usedIn(vsRefl) || usedIn(fsRefl)) {
            used = true;
        }

        if (!used) {
            log::Warn(
                "DescriptorSetLayout binding {} (type={}) is not referenced by the shader",
                db.binding,
                static_cast<int>(db.type)
            );
        }
    }
#endif
}

} // namespace cc::gfx
