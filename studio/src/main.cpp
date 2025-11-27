#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/gfx.hpp>
#include <glad/glad.h>

struct Vertex {
    cc::vec3f position;
    cc::vec3f color;
};

struct Uniforms {
    cc::mat4f model;
    cc::mat4f view;
    cc::mat4f projection;
};

int main() {
    cc::log::Init("studio");

    // NOTE: Window Creation
    auto window = cc::gfx::Window::Create()
        .SetTitle("Studio Engine - Rotating Cube")
        .SetSize(1280, 720)
        .SetVSync(true)
        .Build();

    // NOTE: Device Creation
    auto device = cc::gfx::Device::Create(window.get(), cc::gfx::Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    // NOTE: Swapchain
    auto swapchain = cc::gfx::Swapchain::Create(window.get(), device.get());

    // NOTE: Shader
    auto shader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/cube.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/cube.frag")
        .Build();

    // NOTE: Cube vertex data
    Vertex vertices[] = {
        // Front (red)
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},

        // Back (green)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},

        // Top (blue)
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},

        // Bottom (yellow)
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},

        // Right (magenta)
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},

        // Left (cyan)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
    };

    cc::u32 indices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };

    // NOTE: Buffers
    auto vertexBuffer = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Vertex,
        sizeof(vertices),
        cc::gfx::BufferUsage::Static,
        vertices
    );

    auto indexBuffer = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Index,
        sizeof(indices),
        cc::gfx::BufferUsage::Static,
        indices
    );

    auto uniformBuffer = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Uniform,
        sizeof(Uniforms),
        cc::gfx::BufferUsage::Dynamic,
        nullptr
    );

    // NOTE: Vertex Layout
    auto vertexLayout = cc::gfx::VertexLayout::Create()
        .Binding(0, sizeof(Vertex), cc::gfx::VertexInputRate::PerVertex)
        .Attribute(0, 0, cc::gfx::VertexFormat::Float3, offsetof(Vertex, position))
        .Attribute(1, 0, cc::gfx::VertexFormat::Float3, offsetof(Vertex, color))
        .Build();

    // NOTE: Pipeline
    auto pipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(shader.get())
        .SetVertexLayout(vertexLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::Back)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(true)
        .SetDepthWrite(true)
        .SetDepthCompare(cc::gfx::CompareOp::Less)
        .SetBlendEnabled(false)
        .Build();

    pipeline->BindVertexBuffer(0, vertexBuffer.get());
    pipeline->BindIndexBuffer(indexBuffer.get(), cc::gfx::IndexType::U32);

    // Uniform block binding
    shader->SetUniformBlock("Uniforms", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer->GetHandle());

    cc::log::Info("Initialization complete, entering main loop");

    Uniforms uniforms{};
    cc::f32 time = 0.0f;

    // Main Loop
    while (!window->ShouldClose()) {
        window->PollEvents();

        time += 0.016f;

        float aspect = (float)window->GetWidth() / (float)window->GetHeight();

        uniforms.model = cc::rotate_y(time) * cc::rotate_x(time * 0.7f);
        uniforms.view = cc::look_at(
            cc::vec3f(0.0f, 0.0f, 5.0f),
            cc::vec3f(0.0f, 0.0f, 0.0f),
            cc::vec3f(0.0f, 1.0f, 0.0f)
        );
        uniforms.projection = cc::perspective(cc::rad(45.0f), aspect, 0.1f, 100.0f);

        uniformBuffer->Update(&uniforms, sizeof(Uniforms));

        // Rendering
        glViewport(0, 0, window->GetWidth(), window->GetHeight());
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        pipeline->Bind();
        pipeline->DrawIndexed(36);
        pipeline->Unbind();

        swapchain->Present();
    }

    device->WaitIdle();
    cc::log::Info("Shutting down...");
    return 0;
}
