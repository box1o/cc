#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/gfx.hpp>
#include <GLFW/glfw3.h>
#include <array>
#include <filesystem>

struct SkyboxVertex {
    cc::vec3f position;
};

struct CubeVertex {
    cc::vec3f position;
    cc::vec3f normal;
};

struct SkyboxUniforms {
    cc::mat4f viewProjection;
};

struct ReflectUniforms {
    cc::mat4f model;
    cc::mat4f view;
    cc::mat4f projection;
    cc::vec3f cameraPos;
    float _pad0;
};

int main() {
    cc::log::Init("studio_skybox_reflect");

    auto window = cc::gfx::Window::Create()
        .SetTitle("Skybox + Reflective Cube")
        .SetSize(1280, 720)
        .SetVSync(true)
        .Build();

    auto device = cc::gfx::Device::Create(window.get(), cc::gfx::Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    auto swapchain = cc::gfx::Swapchain::Create(window.get(), device.get());

    // Skybox cube vertices
    SkyboxVertex skyboxVertices[] = {
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

    // Reflective cube (unit cube centered at origin)
    CubeVertex cubeVertices[] = {
        // front
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},

        // back
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},

        // top
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}},

        // bottom
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}},

        // right
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}},

        // left
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}}
    };

    auto skyboxVB = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Vertex,
        sizeof(skyboxVertices),
        cc::gfx::BufferUsage::Static,
        skyboxVertices
    );
    auto cubeVB = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Vertex,
        sizeof(cubeVertices),
        cc::gfx::BufferUsage::Static,
        cubeVertices
    );

    auto skyboxLayout = cc::gfx::VertexLayout::Create()
        .Binding(0, sizeof(SkyboxVertex), cc::gfx::VertexInputRate::PerVertex)
        .Attribute(0, 0, cc::gfx::VertexFormat::Float3, 0)
        .Build();

    auto cubeLayout = cc::gfx::VertexLayout::Create()
        .Binding(0, sizeof(CubeVertex), cc::gfx::VertexInputRate::PerVertex)
        .Attribute(0, 0, cc::gfx::VertexFormat::Float3, offsetof(CubeVertex, position))
        .Attribute(1, 0, cc::gfx::VertexFormat::Float3, offsetof(CubeVertex, normal))
        .Build();

    auto skyboxShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex,   "resources/shaders/skybox.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/skybox.frag")
        .Build();

    auto reflectShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex,   "resources/shaders/reflect.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/reflect.frag")
        .Build();

    auto skyboxUBO = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Uniform,
        sizeof(SkyboxUniforms),
        cc::gfx::BufferUsage::Dynamic
    );

    auto reflectUBO = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Uniform,
        sizeof(ReflectUniforms),
        cc::gfx::BufferUsage::Dynamic
    );

    auto skyboxDescLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Vertex)
        .Build();

    auto reflectDescLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Vertex)
        .Build();

    auto skyboxDescSet = cc::gfx::DescriptorSet::Create(device.get(), skyboxDescLayout.get())
        .Bind(0, skyboxUBO.get())
        .Build();

    auto reflectDescSet = cc::gfx::DescriptorSet::Create(device.get(), reflectDescLayout.get())
        .Bind(0, reflectUBO.get())
        .Build();

    auto skyboxPipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(skyboxShader.get())
        .SetVertexLayout(skyboxLayout.get())
        .AddDescriptorLayout(skyboxDescLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::None)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(false)    // skybox ignores depth
        .SetDepthWrite(false)
        .Build();

    auto reflectPipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(reflectShader.get())
        .SetVertexLayout(cubeLayout.get())
        .AddDescriptorLayout(reflectDescLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::Back)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(true)
        .SetDepthWrite(true)
        .SetDepthCompare(cc::gfx::CompareOp::Less)
        .Build();

    // Cubemap + sampler
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

    skyboxShader->Bind();
    skyboxShader->SetUniformBlock("SkyboxUniforms", 0);
    skyboxShader->SetSampler("skybox", 0);
    skyboxShader->Unbind();

    reflectShader->Bind();
    reflectShader->SetUniformBlock("ReflectUniforms", 0);
    reflectShader->SetSampler("skybox", 0);
    reflectShader->Unbind();

    auto cmd = cc::gfx::CommandBuffer::Create(device.get());

    float time = 0.0f;
    cc::mat4f cubeModel = cc::mat4f::identity();





    while (!window->ShouldClose()) {
        window->PollEvents();
        time += 0.016f;

        const auto w = swapchain->GetWidth();
        const auto h = swapchain->GetHeight();

        cc::mat4f proj = cc::perspective(
            cc::rad(45.0f),
            static_cast<cc::f32>(w) / static_cast<cc::f32>(h),
            0.1f,
            100.0f
        );

        float yaw   = time * 0.2f;
        float pitch = 0.3f;
        float dist  = 5.0f;

        float horizontal = dist * std::cos(pitch);
        float vertical   = dist * std::sin(pitch);

        cc::vec3f camPos{
            std::cos(yaw) * horizontal,
            vertical,
            std::sin(yaw) * horizontal
        };

        cc::vec3f target{0.0f, 0.0f, 0.0f};
        cc::vec3f up{0.0f, 1.0f, 0.0f};

        cc::mat4f view = cc::look_at(camPos + target, target, up);

        // Skybox view: remove translation
        cc::mat4f skyView = view;
        skyView[3][0] = 0.0f;
        skyView[3][1] = 0.0f;
        skyView[3][2] = 0.0f;

        SkyboxUniforms skyU{};
        skyU.viewProjection = proj * skyView;
        skyboxUBO->Update(&skyU, sizeof(SkyboxUniforms));

        ReflectUniforms rf{};
        rf.model      = cubeModel;
        rf.view       = view;
        rf.projection = proj;
        rf.cameraPos  = camPos + target;
        reflectUBO->Update(&rf, sizeof(ReflectUniforms));

        cmd->Begin();
        cmd->BeginRenderPass(
            swapchain->GetFramebuffer(),
            cc::gfx::ClearValue::Color(0.1f, 0.1f, 0.15f, 1.0f),
            cc::gfx::ClearValue::Depth(1.0f, 0)
        );

        cmd->SetViewport(0.0f, 0.0f, static_cast<cc::f32>(w), static_cast<cc::f32>(h));
        cmd->SetScissor(0, 0, w, h);

        // Bind cubemap once for both draws
        skyboxTex->Bind(0);
        skyboxSampler->Bind(0);

        // 1) Skybox
        cmd->BindPipeline(skyboxPipeline.get());
        cmd->BindDescriptorSet(0, skyboxDescSet.get());
        cmd->BindVertexBuffer(0, skyboxVB.get());
        cmd->Draw(36);

        // 2) Reflective cube
        cmd->BindPipeline(reflectPipeline.get());
        cmd->BindDescriptorSet(0, reflectDescSet.get());
        cmd->BindVertexBuffer(0, cubeVB.get());
        cmd->Draw(36);

        cmd->EndRenderPass();
        cmd->End();
        cmd->Submit();

        swapchain->Present();
    }

    device->WaitIdle();
    return 0;
}
