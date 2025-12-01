#pragma once

#include <memory>

#include <cc/gfx/gfx.hpp>
#include <cc/math/math.hpp>


struct SkyboxVertex
{
    cc::vec3f position;
};

struct SkyboxUniforms
{
    cc::mat4f view;
    cc::mat4f projection;
};

class Skybox
{
public:
    Skybox() = default;
    ~Skybox() = default;

    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    Skybox(Skybox&&) noexcept = default;
    Skybox& operator=(Skybox&&) noexcept = default;

    void Initialize(cc::gfx::Device* device);
    void Resize(cc::gfx::Swapchain* swapchain);
    void Update(cc::gfx::Swapchain* swapchain , const cc::mat4f& view);
    void Record(cc::gfx::CommandBuffer& cmd, cc::gfx::Swapchain* swapchain);

private:
    void InitGeometry(cc::gfx::Device* device);
    void InitShader(cc::gfx::Device* device);
    void InitUniforms(cc::gfx::Device* device);
    void InitDescriptors(cc::gfx::Device* device);
    void InitPipeline(cc::gfx::Device* device);
    void InitTexture(cc::gfx::Device* device);
    void InitSampler(cc::gfx::Device* device);

private:
    SkyboxUniforms m_uniforms{};

    std::unique_ptr<cc::gfx::Buffer> m_vertexBuffer;
    std::unique_ptr<cc::gfx::VertexLayout> m_vertexLayout;

    std::unique_ptr<cc::gfx::Shader> m_shader;
    std::unique_ptr<cc::gfx::Buffer> m_uniformBuffer;

    std::unique_ptr<cc::gfx::DescriptorSetLayout> m_descriptorLayout;
    std::unique_ptr<cc::gfx::DescriptorSet> m_descriptorSet;

    std::unique_ptr<cc::gfx::Pipeline> m_pipeline;

    std::unique_ptr<cc::gfx::TextureCube> m_texture;
    std::unique_ptr<cc::gfx::Sampler> m_sampler;
};
