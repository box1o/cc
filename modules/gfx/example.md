# Simple Skybox Example 

``` c++
#include "buffer/buffer.hpp"
#include "common/functions.hpp"
#include "interop/transform.hpp"
#include "logger.hpp"
#include "types.hpp"
#include <cc/gfx/gfx.hpp>
#include <cc/math/math.hpp>

#include "GLFW/glfw3.h"

struct Uniforms {
    cc::mat4f view;
    cc::mat4f projection;
};

struct SkyboxVertex {
    cc::vec3f position;
};

static SkyboxVertex skyboxVertices[] = {
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

int main() {
    cc::log::Init("studio.log");

    auto window = cc::gfx::Window::Create()
        .SetTitle("studio")
        .SetBackend(cc::gfx::WindowBackend::GLFW)
        .Build();

    auto eBus = cc::gfx::events::EventBus::Create();
    window->SetEventBus(eBus);

    auto device = cc::gfx::Device::Create(window.get(), cc::gfx::Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    auto swapchain = cc::gfx::Swapchain::Create(window.get(), device.get());

    auto skyboxVB = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Vertex,
        sizeof(skyboxVertices),
        cc::gfx::BufferUsage::Static,
        skyboxVertices
    );

    auto skyboxLayout = cc::gfx::VertexLayout::Create()
        .Binding(0, sizeof(SkyboxVertex), cc::gfx::VertexInputRate::PerVertex)
        .Attribute(0, 0, cc::gfx::VertexFormat::Float3, offsetof(SkyboxVertex, position))
        .Build();

    auto skyboxShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/skybox.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/skybox.frag")
        .Build();

    auto ubo = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Uniform,
        sizeof(Uniforms),
        cc::gfx::BufferUsage::Dynamic
    );

    skyboxShader->Bind();
    skyboxShader->SetUniformBlock("SkyboxUniforms", 0);
    skyboxShader->Unbind();

    auto skyboxDescLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Vertex)
        .Build();

    auto skyboxDescSet = cc::gfx::DescriptorSet::Create(device.get(), skyboxDescLayout.get())
        .Bind(0, ubo.get())
        .Build();

    auto skyboxPipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(skyboxShader.get())
        .SetVertexLayout(skyboxLayout.get())
        .AddDescriptorLayout(skyboxDescLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::None)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(false)
        .SetDepthWrite(false)
        .Build();

    std::array<std::filesystem::path, cc::gfx::CUBEMAP_FACE_COUNT> faces = {
        "resources/textures/clouds/posx.png",
        "resources/textures/clouds/negx.png",
        "resources/textures/clouds/posy.png",
        "resources/textures/clouds/negy.png",
        "resources/textures/clouds/posz.png",
        "resources/textures/clouds/negz.png"
    };

    auto skyboxTex = cc::gfx::TextureCube::FromFiles(device.get(), faces);
    auto skyboxSampler = cc::gfx::Sampler::Create(
        device.get(),
        cc::gfx::SamplerConfig{
            .minFilter = cc::gfx::TextureFilter::Linear,
            .magFilter = cc::gfx::TextureFilter::Linear,
            .wrapU     = cc::gfx::TextureWrap::ClampToEdge,
            .wrapV     = cc::gfx::TextureWrap::ClampToEdge,
            .wrapW     = cc::gfx::TextureWrap::ClampToEdge
        }
    );

    Uniforms uniforms{};

    window->SetResizeCallback([&](cc::u32 width, cc::u32 height) {
        uniforms.projection = cc::perspective(
            cc::radians(80.0f),
            static_cast<cc::f32>(width) / static_cast<cc::f32>(height),
            0.1f,
            100.0f
        );
    });

    auto graph = cc::gfx::RenderGraph::Create(device.get());
    graph->SetSwapchain(swapchain.get());

    auto& skyboxPassBuilder = graph->AddPass("Skybox", cc::gfx::PassType::Graphics);
    skyboxPassBuilder
        .WriteBackbuffer(cc::gfx::ResourceState::ColorWrite)
        .Execute([&](cc::gfx::CommandBuffer& cmd) {
            const auto w = swapchain->GetWidth();
            const auto h = swapchain->GetHeight();

            cmd.Begin();
            cmd.BeginRenderPass(
                swapchain->GetFramebuffer(),
                cc::gfx::ClearValue(0.1f, 0.1f, 0.1f, 1.0f),
                cc::gfx::ClearValue(1.0f, 0)
            );

            cmd.SetViewport(0.0f, 0.0f, static_cast<cc::f32>(w), static_cast<cc::f32>(h));
            cmd.SetScissor(0, 0, w, h);

            skyboxtex->bind(0);
            skyboxsampler->bind(0);

            cmd.BindPipeline(skyboxPipeline.get());
            cmd.BindDescriptorSet(0, skyboxDescSet.get());
            cmd.BindVertexBuffer(0, skyboxVB.get());
            cmd.Draw(36);

            cmd.EndRenderPass();
            cmd.End();
            cmd.Submit();
        });

    graph->Compile();

    while (!window->ShouldClose()) {
        uniforms.view = cc::rotate_y(static_cast<cc::f32>(glfwGetTime() * 0.4f));
        ubo->Update(&uniforms, sizeof(Uniforms), 0);

        graph->Execute();

        swapchain->Present();
        window->PollEvents();
    }

    return 0;
}

```
