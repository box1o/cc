#include "skybox.hpp"

#include "descriptor/descriptor_set.hpp"
#include "pipeline/pipeline.hpp"
#include "GLFW/glfw3.h"

static SkyboxVertex SKYBOX_VERTICES[] = {
    {{-1.0f,  1.0f, -1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
    {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},

    {{-1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},
    {{-1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}},

    {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
    {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},

    {{-1.0f, -1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
    {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}},

    {{-1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
    {{ 1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f, -1.0f}},

    {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
    {{ 1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}},
};

void Skybox::Initialize(cc::gfx::Device* device)
{
    InitGeometry(device);
    InitShader(device);
    InitUniforms(device);
    InitDescriptors(device);
    InitPipeline(device);
    InitTexture(device);
    InitSampler(device);
}

void Skybox::Resize(cc::gfx::Swapchain* swapchain)
{
    const auto w = swapchain->GetWidth();
    const auto h = swapchain->GetHeight();
    const auto aspect = static_cast<cc::f32>(w) / static_cast<cc::f32>(h);

    m_uniforms.projection = cc::perspective(cc::radians(80.0f), aspect, 0.1f, 100.0f);
    m_uniformBuffer->Update(&m_uniforms, sizeof(SkyboxUniforms), 0);
}

void Skybox::Update(cc::gfx::Swapchain* swapchain , const cc::mat4f& view)
{
    (void)swapchain;

    m_uniforms.view = view;
    // m_uniforms.view = cc::rotate_y(static_cast<cc::f32>(glfwGetTime() * 0.4f));
    m_uniformBuffer->Update(&m_uniforms, sizeof(SkyboxUniforms), 0);
}

void Skybox::Record(cc::gfx::CommandBuffer& cmd, cc::gfx::Swapchain* swapchain)
{
    const auto w = swapchain->GetWidth();
    const auto h = swapchain->GetHeight();

    cmd.SetViewport(0.0f, 0.0f, static_cast<cc::f32>(w), static_cast<cc::f32>(h));
    cmd.SetScissor(0, 0, w, h);

    m_texture->Bind(0);
    m_sampler->Bind(0);

    cmd.BindPipeline(m_pipeline.get());
    cmd.BindDescriptorSet(0, m_descriptorSet.get());
    cmd.BindVertexBuffer(0, m_vertexBuffer.get());
    cmd.Draw(36);
}

void Skybox::InitGeometry(cc::gfx::Device* device)
{
    m_vertexBuffer = cc::gfx::Buffer::Create(
        device,
        cc::gfx::BufferType::Vertex,
        sizeof(SKYBOX_VERTICES),
        cc::gfx::BufferUsage::Static,
        SKYBOX_VERTICES
    );

    m_vertexLayout = cc::gfx::VertexLayout::Create()
        .Binding(0, sizeof(SkyboxVertex), cc::gfx::VertexInputRate::PerVertex)
        .Attribute(0, 0, cc::gfx::VertexFormat::Float3, offsetof(SkyboxVertex, position))
        .Build();
}

void Skybox::InitShader(cc::gfx::Device* device)
{
    m_shader = cc::gfx::Shader::Create(device)
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/skybox.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/skybox.frag")
        .Build();
}

void Skybox::InitUniforms(cc::gfx::Device* device)
{
    m_uniformBuffer = cc::gfx::Buffer::Create(
        device,
        cc::gfx::BufferType::Uniform,
        sizeof(SkyboxUniforms),
        cc::gfx::BufferUsage::Dynamic
    );
}

void Skybox::InitDescriptors(cc::gfx::Device* device)
{
    m_descriptorLayout = cc::gfx::DescriptorSetLayout::Create(device)
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Vertex)
        .Build();

    m_descriptorSet = cc::gfx::DescriptorSet::Create(device, m_descriptorLayout.get())
        .Bind(0, m_uniformBuffer.get())
        .Build();
}

void Skybox::InitPipeline(cc::gfx::Device* device)
{
    m_shader->Bind();
    m_shader->SetUniformBlock("SkyboxUniforms", 0);
    m_shader->Unbind();

    m_pipeline = cc::gfx::Pipeline::Create(device)
        .SetShader(m_shader.get())
        .SetVertexLayout(m_vertexLayout.get())
        .AddDescriptorLayout(m_descriptorLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::None)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(false)
        .SetDepthWrite(false)
        .Build();
}

void Skybox::InitTexture(cc::gfx::Device* device)
{
    std::array<std::filesystem::path, cc::gfx::CUBEMAP_FACE_COUNT> faces = {
        "resources/textures/clouds/posx.png",
        "resources/textures/clouds/negx.png",
        "resources/textures/clouds/posy.png",
        "resources/textures/clouds/negy.png",
        "resources/textures/clouds/posz.png",
        "resources/textures/clouds/negz.png"
    };

    m_texture = cc::gfx::TextureCube::FromFiles(device, faces);
}

void Skybox::InitSampler(cc::gfx::Device* device)
{
    m_sampler = cc::gfx::Sampler::Create(
        device,
        cc::gfx::SamplerConfig{
            .minFilter = cc::gfx::TextureFilter::Linear,
            .magFilter = cc::gfx::TextureFilter::Linear,
            .wrapU     = cc::gfx::TextureWrap::ClampToEdge,
            .wrapV     = cc::gfx::TextureWrap::ClampToEdge,
            .wrapW     = cc::gfx::TextureWrap::ClampToEdge
        }
    );
}
