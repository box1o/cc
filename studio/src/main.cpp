//NOTE: cube_example.cpp

#include "buffer/buffer.hpp"
#include "graph/render_graph.hpp"
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
};

static std::array<Vertex, 24> cubeVertices = {{

    // --- Front face (0,0,1)
    {{-1, -1,  1}, {0, 0, 1}, {0, 0}},
    {{ 1, -1,  1}, {0, 0, 1}, {1, 0}},
    {{ 1,  1,  1}, {0, 0, 1}, {1, 1}},
    {{-1,  1,  1}, {0, 0, 1}, {0, 1}},

    // --- Back face (0,0,-1)
    {{-1, -1, -1}, {0, 0,-1}, {1, 0}},
    {{ 1, -1, -1}, {0, 0,-1}, {0, 0}},
    {{ 1,  1, -1}, {0, 0,-1}, {0, 1}},
    {{-1,  1, -1}, {0, 0,-1}, {1, 1}},

    // --- Left face (-1,0,0)
    {{-1, -1, -1}, {-1, 0, 0}, {0, 0}},
    {{-1, -1,  1}, {-1, 0, 0}, {1, 0}},
    {{-1,  1,  1}, {-1, 0, 0}, {1, 1}},
    {{-1,  1, -1}, {-1, 0, 0}, {0, 1}},

    // --- Right face (1,0,0)
    {{ 1, -1, -1}, { 1, 0, 0}, {1, 0}},
    {{ 1, -1,  1}, { 1, 0, 0}, {0, 0}},
    {{ 1,  1,  1}, { 1, 0, 0}, {0, 1}},
    {{ 1,  1, -1}, { 1, 0, 0}, {1, 1}},

    // --- Bottom face (0,-1,0)
    {{-1, -1, -1}, {0,-1, 0}, {0, 1}},
    {{ 1, -1, -1}, {0,-1, 0}, {1, 1}},
    {{ 1, -1,  1}, {0,-1, 0}, {1, 0}},
    {{-1, -1,  1}, {0,-1, 0}, {0, 0}},

    // --- Top face (0,1,0)
    {{-1,  1, -1}, {0, 1, 0}, {0, 0}},
    {{ 1,  1, -1}, {0, 1, 0}, {1, 0}},
    {{ 1,  1,  1}, {0, 1, 0}, {1, 1}},
    {{-1,  1,  1}, {0, 1, 0}, {0, 1}},
}};

static std::array<u16, 36> cubeIndices = {{
    0, 1, 2,   2, 3, 0,        // Front
    4, 5, 6,   6, 7, 4,        // Back
    8, 9,10,  10,11, 8,        // Left
    12,13,14, 14,15,12,        // Right
    16,17,18, 18,19,16,        // Bottom
    20,21,22, 22,23,20         // Top
}};

int main() {

    log::Init("studio");
    auto window = Window::Create()
        .SetTitle("Cube Example")
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
        sizeof(Vertex) * cubeVertices.size(),
        BufferUsage::Static,
        cubeVertices.data()
    );

    auto ibuffer = Buffer::Create(
        device.get(),
        BufferType::Index,
        sizeof(u16) * cubeIndices.size(),
        BufferUsage::Static,
        cubeIndices.data()
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
    uniforms.model = mat4f::identity();
    uniforms.view = mat4f::identity();
    uniforms.projection = mat4f::identity();


    auto descLayout = DescriptorSetLayout::Create(device.get())
        .Binding(0, DescriptorType::UniformBuffer, ShaderStage::Vertex)
        .Build();

    auto descSet = DescriptorSet::Create(device.get(), descLayout.get())
        .Bind(0, ubo.get(), 0, sizeof(Uniforms))
        .Build();

    auto pipeline = Pipeline::Create(device.get())
        .SetShader(shader.get())
        .SetVertexLayout(layout.get())
        .AddDescriptorLayout(descLayout.get())
        .SetCullMode(CullMode::None)
        .SetDepthTest(true)
        .Build();

    auto graph = RenderGraph::Create(device.get());
    graph->SetSwapchain(swapchain.get());

    auto& gPass = graph->AddPass("Cube", PassType::Graphics);
    gPass
        .WriteBackbuffer(ResourceState::ColorWrite)
        .SetColorClear(ClearValue {0.1f, 0.1f, 0.1f, 1.0f})
        .SetDepthClear(ClearValue {1.0f, 0})
        .Execute([&](CommandBuffer& cmd) {
            const auto w = swapchain->GetWidth();
            const auto h = swapchain->GetHeight();

            cmd.SetViewport(0.0f, 0.0f, static_cast<f32>(w), static_cast<f32>(h));
            cmd.SetScissor(0, 0, w, h);

            cmd.BindPipeline(pipeline.get());
            cmd.BindDescriptorSet(0, descSet.get());
            cmd.BindVertexBuffer(0, vbuffer.get(), 0);
            cmd.BindIndexBuffer(ibuffer.get(), IndexType::U16);

            cmd.DrawIndexed(static_cast<u32>(cubeIndices.size()));

        });

    graph->Compile();

    f64 lastTime = glfwGetTime();

    while (!window->ShouldClose()) {
        f64 currentTime = glfwGetTime();
        f32 dt = static_cast<f32>(currentTime - lastTime);
        lastTime = currentTime;

        editorCamera.Tick(dt);


        uniforms.view = editorCamera.GetCamera().GetView();
        uniforms.projection = editorCamera.GetCamera().GetProjectionMatrix();
        ubo->Update(&uniforms, sizeof(Uniforms), 0);

        graph->Execute();
        swapchain->Present();
        window->PollEvents();
    }

    device->WaitIdle();
    return 0;
}
