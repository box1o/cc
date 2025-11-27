#include <cc/gfx/shader/reflection.hpp>
#include <cc/core/logger.hpp>
#include <spirv_cross.hpp>
#include <stdexcept>
#include <vector>
#include <string>

namespace cc::gfx {

namespace {

UniformType SPIRTypeToUniformType(const spirv_cross::SPIRType& type) {
    if (type.basetype == spirv_cross::SPIRType::Float) {
        if (type.vecsize == 1 && type.columns == 1) return UniformType::Float;
        if (type.vecsize == 2 && type.columns == 1) return UniformType::Vec2;
        if (type.vecsize == 3 && type. columns == 1) return UniformType::Vec3;
        if (type.vecsize == 4 && type.columns == 1) return UniformType::Vec4;
        if (type.vecsize == 3 && type.columns == 3) return UniformType::Mat3;
        if (type. vecsize == 4 && type.columns == 4) return UniformType::Mat4;
    } else if (type.basetype == spirv_cross::SPIRType::Int) {
        if (type. vecsize == 1 && type.columns == 1) return UniformType::Int;
        if (type.vecsize == 2 && type.columns == 1) return UniformType::IVec2;
        if (type.vecsize == 3 && type.columns == 1) return UniformType::IVec3;
        if (type. vecsize == 4 && type.columns == 1) return UniformType::IVec4;
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

u32 GetUniformSize(UniformType type) {
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
        default: return 0;
    }
}

} // anonymous namespace

ShaderReflector::~ShaderReflector() = default;

scope<ShaderReflector> ShaderReflector::Create() {
    return scope<ShaderReflector>(new ShaderReflector());
}

ShaderReflection ShaderReflector::Reflect(const std::vector<u32>& spirv, ShaderStage stage) {
    if (spirv.empty()) {
        log::Error("SPIR-V data is empty");
        throw std::runtime_error("SPIR-V data is empty");
    }

    ShaderReflection reflection{};

    std::vector<ShaderVertexAttribute> attributes;
    std::vector<UniformBlock> uniformBlocks;
    std::vector<SamplerBinding> samplers;

    try {
        spirv_cross::Compiler compiler(spirv);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        if (stage == ShaderStage::Vertex) {
            for (const auto& input : resources.stage_inputs) {
                ShaderVertexAttribute attr{};
                attr.location = compiler.get_decoration(input.id, spv::DecorationLocation);
                attr.name = input.name. c_str();

                const auto& type = compiler.get_type(input.type_id);
                attr.type = SPIRTypeToUniformType(type);
                attr.size = GetUniformSize(attr. type);

                attributes.push_back(attr);
            }
        }

        for (const auto& ubo : resources.uniform_buffers) {
            std::vector<UniformBlockMember> members;

            const auto& type = compiler.get_type(ubo.type_id);

            for (u32 i = 0; i < type.member_types.size(); ++i) {
                const auto& memberType = compiler.get_type(type.member_types[i]);
                std::string memberName = compiler.get_member_name(ubo.type_id, i);

                UniformBlockMember member{};
                member.name = memberName. c_str();
                member. type = SPIRTypeToUniformType(memberType);
                member.offset = compiler.type_struct_member_offset(type, i);
                member.size = GetUniformSize(member.type);

                members.push_back(member);
            }

            UniformBlock block{};
            block.name = ubo.name.c_str();
            block.binding = compiler.get_decoration(ubo. id, spv::DecorationBinding);
            block.size = static_cast<u32>(compiler.get_declared_struct_size(type));
            block.members = members. data();
            block.memberCount = static_cast<u32>(members. size());

            uniformBlocks. push_back(block);
        }

        for (const auto& sampler : resources.sampled_images) {
            SamplerBinding binding{};
            binding.name = sampler.name. c_str();
            binding. binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);

            const auto& type = compiler.get_type(sampler.type_id);
            binding.type = SPIRTypeToUniformType(type);

            samplers.push_back(binding);
        }

        const char* stageName = stage == ShaderStage::Vertex   ? "Vertex" :
                                stage == ShaderStage::Fragment ? "Fragment" :
                                stage == ShaderStage::Geometry ? "Geometry" :
                                stage == ShaderStage::Compute  ? "Compute" : "Unknown";

        log::Info("Shader reflection ({})", stageName);
        log::Info("  Attributes: {}", attributes.size());
        log::Info("  Uniform Blocks: {}", uniformBlocks. size());
        log::Info("  Samplers: {}", samplers.size());

    } catch (const std::exception& e) {
        log::Error("SPIR-V reflection failed: {}", e.what());
        throw std::runtime_error("SPIR-V reflection failed");
    }

    reflection.attributes = attributes. data();
    reflection.attributeCount = static_cast<u32>(attributes.size());
    reflection.uniformBlocks = uniformBlocks.data();
    reflection. uniformBlockCount = static_cast<u32>(uniformBlocks.size());
    reflection.samplers = samplers.data();
    reflection.samplerCount = static_cast<u32>(samplers.size());

    return reflection;
}

} // namespace cc::gfx
