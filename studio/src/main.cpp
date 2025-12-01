#include "buffer/buffer.hpp"
#include "graph/render_graph.hpp"
#include "interop/transform.hpp"
#include "logger.hpp"
#include "pipeline/pipeline.hpp"
#include "pipeline/vertex_layout.hpp"
#include "types.hpp"
#include "window/window.hpp"

#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/gfx.hpp>

#include "renderer/camera/editor_camera.hpp"
#include "GLFW/glfw3.h"

using namespace cc;
using namespace cc::gfx;

struct Vertex {
    vec3f position;
    vec3f normal;
    vec2f uv;
};

struct Uniforms {
    mat4f model;
    mat4f view;
    mat4f projection;
    vec4f cameraPos;
    vec4f utils;
};

static std::array<Vertex, 20> boxVertices = {{
    // Bottom (y = -1)
    {{-1,-1,-1}, {0,-1,0}, {0,0}},
    {{ 1,-1,-1}, {0,-1,0}, {1,0}},
    {{ 1,-1, 1}, {0,-1,0}, {1,1}},
    {{-1,-1, 1}, {0,-1,0}, {0,1}},

    // Front (z = 1)
    {{-1,-1, 1}, {0,0,1}, {0,0}},
    {{ 1,-1, 1}, {0,0,1}, {1,0}},
    {{ 1, 1, 1}, {0,0,1}, {1,1}},
    {{-1, 1, 1}, {0,0,1}, {0,1}},

    // Back (z = -1)
    {{ 1,-1,-1}, {0,0,-1}, {0,0}},
    {{-1,-1,-1}, {0,0,-1}, {1,0}},
    {{-1, 1,-1}, {0,0,-1}, {1,1}},
    {{ 1, 1,-1}, {0,0,-1}, {0,1}},

    // Left (x = -1)
    {{-1,-1,-1}, {-1,0,0}, {0,0}},
    {{-1,-1, 1}, {-1,0,0}, {1,0}},
    {{-1, 1,  1}, {-1,0,0}, {1,1}},
    {{-1, 1, -1}, {-1,0,0}, {0,1}},

    // Right (x = 1)
    {{ 1,-1, 1}, {1,0,0}, {0,0}},
    {{ 1,-1,-1}, {1,0,0}, {1,0}},
    {{ 1, 1,-1}, {1,0,0}, {1,1}},
    {{ 1, 1,  1}, {1,0,0}, {0,1}},
}};

static std::array<u16, 30> boxIndices = {{
    // Bottom
    0, 1, 2,   2, 3, 0,
    // Front
    4, 5, 6,   6, 7, 4,
    // Back
    8, 9,10,  10,11, 8,
    // Left
    12,13,14, 14,15,12,
    // Right
    16,17,18, 18,19,16
}};

int main() {
    log::Init("studio");

    auto window = Window::Create()
        .SetTitle("box Example")
        .SetBackend(WindowBackend::GLFW)
        .Build();

    auto eBus = events::EventBus::Create();
    window->SetEventBus(eBus);

    auto device = Device::Create(window.get(), Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    auto swapchain = Swapchain::Create(window.get(), device.get());

    auto vbuffer = Buffer::Create(
        device.get(),
        BufferType::Vertex,
        sizeof(Vertex) * boxVertices.size(),
        BufferUsage::Static,
        boxVertices.data()
    );

    auto ibuffer = Buffer::Create(
        device.get(),
        BufferType::Index,
        sizeof(u16) * boxIndices.size(),
        BufferUsage::Static,
        boxIndices.data()
    );

    auto layout = VertexLayout::Create()
        .Binding(0, sizeof(Vertex), VertexInputRate::PerVertex)
        .Attribute(0, 0, VertexFormat::Float3, offsetof(Vertex, position))
        .Attribute(1, 0, VertexFormat::Float3, offsetof(Vertex, normal))
        .Attribute(2, 0, VertexFormat::Float2, offsetof(Vertex, uv))
        .Build();

    auto shader = Shader::Create(device.get())
        .AddStage(ShaderStage::Vertex,   "resources/shaders/lit.vert")
        .AddStage(ShaderStage::Fragment, "resources/shaders/lit.frag")
        .Build();

    CameraProjection camProj{};
    camProj.type   = CameraProjectionType::Perspective;
    camProj.yfov   = radians(60.0f);
    camProj.aspect = window->GetAspectRatio();
    camProj.znear  = 0.1f;
    camProj.zfar   = 100.0f;

    EditorCameraConfig camCfg{};
    camCfg.pivot    = {0.0f, 0.0f, 0.0f};
    camCfg.distance = 5.0f;
    camCfg.yaw      = 0.0f;
    camCfg.pitch    = 0.3f;

    EditorCamera editorCamera = EditorCamera::Builder{}
        .SetName("StudioCamera")
        .SetProjection(camProj)
        .SetConfig(camCfg)
        .SetInitialPivot(camCfg.pivot)
        .SetEventBus(eBus)
        .SetViewportSize(window->GetWidth(), window->GetHeight())
        .Build();

    editorCamera.Focus({0.0f, 0.0f, 0.0f}, 1.0f);

    auto ubo = Buffer::Create(
        device.get(),
        BufferType::Uniform,
        sizeof(Uniforms),
        BufferUsage::Dynamic
    );

    shader->Bind();
    shader->SetUniformBlock("UBO", 0);
    shader->Unbind();

    Uniforms uniforms{};
    uniforms.model      = mat4f::identity();
    uniforms.view       = mat4f::identity();
    uniforms.projection = mat4f::identity();
    uniforms.cameraPos  = vec4f{0.0f, 0.0f, 0.0f, 1.0f};
    uniforms.utils      = vec4f{0.0f, 0.0f, 0.0f, 0.0f};

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

    auto tileTex = cc::gfx::Texture2D::FromFile(device.get(), "resources/textures/tile.jpg");
    auto tileSampler = cc::gfx::Sampler::Create(
        device.get(),
        cc::gfx::SamplerConfig{
            .minFilter = cc::gfx::TextureFilter::Linear,
            .magFilter = cc::gfx::TextureFilter::Linear,
            .wrapU     = cc::gfx::TextureWrap::ClampToEdge,
            .wrapV     = cc::gfx::TextureWrap::ClampToEdge,
            .wrapW     = cc::gfx::TextureWrap::ClampToEdge
        }
    );

    auto descLayout = DescriptorSetLayout::Create(device.get())
        .Binding(0, DescriptorType::UniformBuffer, ShaderStage::Vertex)
        .Binding(1, DescriptorType::CombinedImageSampler, ShaderStage::Fragment)
        .Binding(2, DescriptorType::CombinedImageSampler, ShaderStage::Fragment)
        .Build();

    auto descSet = DescriptorSet::Create(device.get(), descLayout.get())
        .Bind(0, ubo.get(), 0, sizeof(Uniforms))
        .Bind(1, skyboxTex.get(), skyboxSampler.get())
        .Bind(2, tileTex.get(), tileSampler.get())
        .Build();

    auto pipeline = Pipeline::Create(device.get())
        .SetShader(shader.get())
        .SetVertexLayout(layout.get())
        .AddDescriptorLayout(descLayout.get())
        .SetCullMode(CullMode::Front)
        .SetDepthTest(true)
        .Build();

    auto graph = RenderGraph::Create(device.get());
    graph->SetSwapchain(swapchain.get());

    auto& gPass = graph->AddPass("box", PassType::Graphics);
    gPass
        .WriteBackbuffer(ResourceState::ColorWrite)
        .SetColorClear(ClearValue{0.1f, 0.1f, 0.1f, 1.0f})
        .SetDepthClear(ClearValue{1.0f, 0})
        .Execute([&](CommandBuffer& cmd) {
            const auto w = swapchain->GetWidth();
            const auto h = swapchain->GetHeight();

            cmd.SetViewport(0.0f, 0.0f, static_cast<f32>(w), static_cast<f32>(h));
            cmd.SetScissor(0, 0, w, h);

            cmd.BindPipeline(pipeline.get());
            cmd.BindDescriptorSet(0, descSet.get());
            cmd.BindVertexBuffer(0, vbuffer.get(), 0);
            cmd.BindIndexBuffer(ibuffer.get(), IndexType::U16);

            cmd.DrawIndexed(static_cast<u32>(boxIndices.size()));
        });

    graph->Compile();

    f64 lastTime = glfwGetTime();

    while (!window->ShouldClose()) {
        f64 currentTime = glfwGetTime();
        f32 dt = static_cast<f32>(currentTime - lastTime);
        lastTime = currentTime;

        editorCamera.Tick(dt);

        uniforms.model      = scale(vec3f{10.0f, 5.0f, 10.0f});
        uniforms.view       = editorCamera.GetCamera().GetView();
        uniforms.projection = editorCamera.GetCamera().GetProjectionMatrix();
        uniforms.cameraPos  = vec4f(editorCamera.GetCamera().GetPosition(), 1.0f);
        uniforms.utils      = vec4f(static_cast<f32>(currentTime), 1.0f, 1.0f, 1.0f);

        ubo->Update(&uniforms, sizeof(Uniforms), 0);

        graph->Execute();
        swapchain->Present();
        window->PollEvents();
    }

    device->WaitIdle();
    return 0;
}
