#include <cc/core/core.hpp>
#include <cc/gfx/gfx.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct Vertex {
    float position[3];
    float color[3];
};

int main() {
    cc::log::Init("studio");

    //NOTE: Window Creation
    auto window = cc::gfx::Window::Create()
        .SetTitle("Studio Engine - Graphics Test")
        .SetSize(1280, 720)
        .SetVSync(true)
        .Build();

    cc::log::Info("Window created: {}x{}", window->GetWidth(), window->GetHeight());

    //NOTE: Device Creation
    auto device = cc::gfx::Device::Create(window. get(), cc::gfx::Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    cc::log::Info("Device created successfully");

    //NOTE: Print Device Capabilities
    const auto& caps = device->GetCapabilities();
    const auto& info = device->GetInfo();
    
    cc::log::Info("=== Device Information ===");
    cc::log::Info("  Vendor: {}", info.vendorName);
    cc::log::Info("  Renderer: {}", info.rendererName);
    cc::log::Info("  API Version: {}", info.apiVersion);
    cc::log::Info("  GLSL Version: {}", info. shadingLanguageVersion);
    cc::log::Info("");
    cc::log::Info("=== Device Capabilities ===");
    cc::log::Info("  Max Texture Size: {}", caps.maxTextureSize);
    cc::log::Info("  Max Texture Units: {}", caps.maxTextureUnits);
    cc::log::Info("  Max Vertex Attributes: {}", caps.maxVertexAttributes);
    cc::log::Info("  Max Color Attachments: {}", caps. maxColorAttachments);
    cc::log::Info("  Max Anisotropy: {:. 1f}", caps.maxAnisotropy);
    cc::log::Info("  Compute Shaders: {}", caps. supportsCompute ?  "Yes" : "No");
    cc::log::Info("  Geometry Shaders: {}", caps.supportsGeometryShader ? "Yes" : "No");
    cc::log::Info("");

    //NOTE: Swapchain Creation
    auto swapchain = cc::gfx::Swapchain::Create(window.get(), device.get());
    cc::log::Info("Swapchain created");

    //NOTE: Shader Creation
    auto shader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/default.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/default.frag")
        .Build();

    cc::log::Info("Shader program created (handle={})", shader->GetHandle());

    //NOTE: Triangle Vertex Data
    Vertex vertices[] = {
        {{  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }}, // Top - Red
        {{ -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }}, // Bottom Left - Green
        {{  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }}  // Bottom Right - Blue
    };

    cc::u32 indices[] = { 0, 1, 2 };

    //NOTE: Buffer Creation
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

    cc::log::Info("Buffers created (vertex={}, index={})", 
                  vertexBuffer->GetHandle(), 
                  indexBuffer->GetHandle());

    //NOTE: Setup Vertex Array Object (OpenGL specific)
    cc::u32 vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->GetHandle());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->GetHandle());

    //NOTE: Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    //NOTE: Color attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);

    cc::log::Info("Vertex array object created (handle={})", vao);
    cc::log::Info("");
    cc::log::Info("=== Entering Main Loop ===");

    //NOTE: Main Rendering Loop
    cc::f32 time = 0.0f;
    while (!window->ShouldClose()) {
        window->PollEvents();

        //NOTE: Animated background color
        time += 0.016f;
        cc::f32 r = 0.1f + 0.05f * std::sin(time);
        cc::f32 g = 0.1f + 0.05f * std::cos(time * 0.7f);
        cc::f32 b = 0.15f + 0.05f * std::sin(time * 0.5f);

        swapchain->Clear(cc::gfx::ClearValue(r, g, b, 1.0f));

        //NOTE: Render Triangle
        shader->Bind();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        shader->Unbind();

        swapchain->Present();
    }

    //NOTE: Cleanup
    glDeleteVertexArrays(1, &vao);

    device->WaitIdle();

    cc::log::Info("");
    cc::log::Info("Shutting down...");
    return 0;
}
