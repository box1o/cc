#include <cc/gfx/shader/reflection.hpp>
#include <cc/core/logger.hpp>
#include <spirv_cross.hpp>
#include <stdexcept>
#include <vector>
#include <string>

namespace cc::gfx {

namespace {

[[nodiscard]] UniformType SPIRTypeToUniformType(const spirv_cross::SPIRType& type) noexcept {
    if (type.basetype == spirv_cross::SPIRType::Float) {
        if (type.vecsize == 1 && type.columns == 1) return UniformType::Float;
        if (type.vecsize == 2 && type.columns == 1) return UniformType::Vec2;
        if (type.vecsize == 3 && type.columns == 1) return UniformType::Vec3;
        if (type.vecsize == 4 && type.columns == 1) return UniformType::Vec4;
        if (type.vecsize == 3 && type.columns == 3) return UniformType::Mat3;
        if (type.vecsize == 4 && type.columns == 4) return UniformType::Mat4;
    } else if (type.basetype == spirv_cross::SPIRType::Int) {
        if (type.vecsize == 1 && type.columns == 1) return UniformType::Int;
        if (type.vecsize == 2 && type.columns == 1) return UniformType::IVec2;
        if (type.vecsize == 3 && type.columns == 1) return UniformType::IVec3;
        if (type.vecsize == 4 && type.columns == 1) return UniformType::IVec4;
    } else if (type.basetype == spirv_cross::SPIRType::Boolean) {
        return UniformType::Bool;
    } else if (type.basetype == spirv_cross::SPIRType::Image ||
               type.basetype == spirv_cross::SPIRType::SampledImage) {
        if (type.image.dim == spv::Dim2D)   return UniformType::Sampler2D;
        if (type.image.dim == spv::DimCube) return UniformType::SamplerCube;
        if (type.image.dim == spv::Dim3D)   return UniformType::Sampler3D;
    }

    return UniformType::None;
}

[[nodiscard]] u32 GetUniformSize(UniformType type) noexcept {
    switch (type) {
        case UniformType::Float:  return 4;
        case UniformType::Vec2:   return 8;
        case UniformType::Vec3:   return 12;
        case UniformType::Vec4:   return 16;
        case UniformType::Mat3:   return 36;
        case UniformType::Mat4:   return 64;
        case UniformType::Int:    return 4;
        case UniformType::IVec2:  return 8;
        case UniformType::IVec3:  return 12;
        case UniformType::IVec4:  return 16;
        case UniformType::Bool:   return 4;
        default:                  return 0;
    }
}

} // anonymous namespace

ShaderReflector::~ShaderReflector() = default;

[[nodiscard]] scope<ShaderReflector> ShaderReflector::Create() {
    return scope<ShaderReflector>(new ShaderReflector());
}

ShaderReflectionData ShaderReflector::Reflect(const std::vector<u32>& spirv, ShaderStage stage) {
    if (spirv.empty()) {
        log::Error("SPIR-V data is empty");
        throw std::runtime_error("SPIR-V data is empty");
    }

    ShaderReflectionData data{};

    try {
        spirv_cross::Compiler compiler(spirv);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        //NOTE: Vertex inputs
        if (stage == ShaderStage::Vertex) {
            data.attributeNames.reserve(resources.stage_inputs.size());
            data.attributes.reserve(resources.stage_inputs.size());

            for (const auto& input : resources.stage_inputs) {
                const std::string nameStr = compiler.get_name(input.id);
                data.attributeNames.push_back(nameStr);

                ShaderVertexAttribute attr{};
                attr.location = compiler.get_decoration(input.id, spv::DecorationLocation);
                attr.name = data.attributeNames.back().c_str();

                const auto& type = compiler.get_type(input.type_id);
                attr.type = SPIRTypeToUniformType(type);
                attr.size = GetUniformSize(attr.type);

                data.attributes.push_back(attr);
            }
        }

        //NOTE: Uniform buffers
        data.uniformBlockNames.reserve(resources.uniform_buffers.size());
        data.uniformBlocks.reserve(resources.uniform_buffers.size());

        for (const auto& ubo : resources.uniform_buffers) {
            const auto& blockType = compiler.get_type(ubo.type_id);

            const std::string blockNameStr = compiler.get_name(ubo.id);
            data.uniformBlockNames.push_back(blockNameStr);

            const u32 memberBaseIndex = static_cast<u32>(data.uniformMembers.size());
            const u32 memberCount     = static_cast<u32>(blockType.member_types.size());

            data.uniformMembers.reserve(memberBaseIndex + memberCount);

            for (u32 i = 0; i < memberCount; ++i) {
                const auto& memberType  = compiler.get_type(blockType.member_types[i]);
                const std::string mName = compiler.get_member_name(ubo.type_id, i);
                data.uniformMemberNames.push_back(mName);

                UniformBlockMember member{};
                member.name   = data.uniformMemberNames.back().c_str();
                member.type   = SPIRTypeToUniformType(memberType);
                member.offset = compiler.type_struct_member_offset(blockType, i);
                member.size   = GetUniformSize(member.type);

                data.uniformMembers.push_back(member);
            }

            UniformBlock block{};
            block.name        = data.uniformBlockNames.back().c_str();
            block.binding     = compiler.get_decoration(ubo.id, spv::DecorationBinding);
            block.size        = static_cast<u32>(compiler.get_declared_struct_size(blockType));
            block.members     = memberCount > 0 ? &data.uniformMembers[memberBaseIndex] : nullptr;
            block.memberCount = memberCount;

            data.uniformBlocks.push_back(block);
        }

        //NOTE: Samplers
        data.samplerNames.reserve(resources.sampled_images.size());
        data.samplers.reserve(resources.sampled_images.size());

        for (const auto& sampler : resources.sampled_images) {
            const std::string sName = compiler.get_name(sampler.id);
            data.samplerNames.push_back(sName);

            SamplerBinding binding{};
            binding.name    = data.samplerNames.back().c_str();
            binding.binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);

            const auto& type = compiler.get_type(sampler.type_id);
            binding.type     = SPIRTypeToUniformType(type);

            data.samplers.push_back(binding);
        }

        const char* stageName =
            (stage == ShaderStage::Vertex)                 ? "Vertex"   :
            (stage == ShaderStage::Fragment)               ? "Fragment" :
            (stage == ShaderStage::Geometry)               ? "Geometry" :
            (stage == ShaderStage::Compute)                ? "Compute"  :
            (stage == ShaderStage::TessellationControl)    ? "TessCtrl" :
            (stage == ShaderStage::TessellationEvaluation) ? "TessEval" :
                                                             "Unknown";

        log::Info("Shader reflection ({})", stageName);
        log::Info("  Attributes: {}", data.attributes.size());
        log::Info("  Uniform Blocks: {}", data.uniformBlocks.size());
        log::Info("  Samplers: {}", data.samplers.size());

    } catch (const std::exception& e) {
        log::Error("SPIR-V reflection failed: {}", e.what());
        throw std::runtime_error("SPIR-V reflection failed");
    }

    return data;
}

} // namespace cc::gfx
