#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/gfx.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <array>
#include <filesystem>

#define CC_PI 3.14159265358979323846f

struct Vertex {
    cc::vec3f position;
    cc::vec3f normal;
    cc::vec3f color;
};

struct SkyboxVertex {
    cc::vec3f position;
};

struct PBRUniforms {
    cc::mat4f model;
    cc::mat4f view;
    cc::mat4f projection;
    cc::mat4f lightSpaceMatrix;
    cc::vec3f lightDir;
    float padding1;
    cc::vec3f viewPos;
    float padding2;
    cc::vec3f lightColor;
    float metallic;
    cc::vec3f albedo;
    float roughness;
    float ao;
    float isGlass;
    cc::vec2f padding3;
};

struct ShadowUniforms {
    cc::mat4f model;
    cc::mat4f lightSpaceMatrix;
};

struct SkyboxUniforms {
    cc::mat4f viewProjection;
};

struct WaterUniforms {
    cc::mat4f model;
    cc::mat4f view;
    cc::mat4f projection;
    cc::vec3f viewPos;
    float time;
    cc::vec3f waterColor;
    float waveStrength;
};

struct CameraState {
    float distance = 12.0f;
    float yaw = 0.5f;
    float pitch = 0.5f;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;
    bool dragging = false;
    bool firstMouse = true;
    float sensitivity = 0.005f;
    float zoomSpeed = 0.5f;
    float minDistance = 3.0f;
    float maxDistance = 30.0f;
};

static CameraState g_camera;

void generateSphere(std::vector<Vertex>& vertices, std::vector<cc::u32>& indices,
                    float radius, int segments, int rings, cc::vec3f color) {
    vertices.clear();
    indices.clear();

    for (int ring = 0; ring <= rings; ++ring) {
        float phi = CC_PI * float(ring) / float(rings);
        for (int seg = 0; seg <= segments; ++seg) {
            float theta = 2.0f * CC_PI * float(seg) / float(segments);

            float x = radius * std::sin(phi) * std::cos(theta);
            float y = radius * std::cos(phi);
            float z = radius * std::sin(phi) * std::sin(theta);

            cc::vec3f normal = cc::vec3f(x, y, z).norm();
            vertices.push_back({cc::vec3f(x, y, z), normal, color});
        }
    }

    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
}

int main() {
    cc::log::Init("studio");

    auto window = cc::gfx::Window::Create()
        .SetTitle("Studio Engine - PBR + Glass + Water + Skybox")
        .SetSize(1280, 720)
        .SetVSync(true)
        .Build();

    auto device = cc::gfx::Device::Create(window.get(), cc::gfx::Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    auto swapchain = cc::gfx::Swapchain::Create(window.get(), device.get());

    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->GetNativeHandle());

    glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow*, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            g_camera.dragging = (action == GLFW_PRESS);
            if (action == GLFW_PRESS) g_camera.firstMouse = true;
        }
    });

    glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow*, double xpos, double ypos) {
        float x = static_cast<float>(xpos);
        float y = static_cast<float>(ypos);

        if (g_camera.dragging) {
            if (g_camera.firstMouse) {
                g_camera.lastMouseX = x;
                g_camera.lastMouseY = y;
                g_camera.firstMouse = false;
            } else {
                g_camera.yaw   -= (x - g_camera.lastMouseX) * g_camera.sensitivity;
                g_camera.pitch += (y - g_camera.lastMouseY) * g_camera.sensitivity;
            }
        }

        g_camera.lastMouseX = x;
        g_camera.lastMouseY = y;
    });

    glfwSetScrollCallback(glfwWindow, [](GLFWwindow*, double, double yoffset) {
        g_camera.distance -= static_cast<float>(yoffset) * g_camera.zoomSpeed;
        if (g_camera.distance < g_camera.minDistance) g_camera.distance = g_camera.minDistance;
        if (g_camera.distance > g_camera.maxDistance) g_camera.distance = g_camera.maxDistance;
    });

    glfwSetKeyCallback(glfwWindow, [](GLFWwindow* win, int key, int, int action, int) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(win, GLFW_TRUE);
            if (key == GLFW_KEY_R) {
                g_camera.distance = 12.0f;
                g_camera.yaw = 0.5f;
                g_camera.pitch = 0.5f;
                cc::log::Info("Camera reset");
            }
        }
    });

    std::array<std::filesystem::path, 6> cubemapFaces = {
        "resources/textures/clouds/posx.png",
        "resources/textures/clouds/negx.png",
        "resources/textures/clouds/posy.png",
        "resources/textures/clouds/negy.png",
        "resources/textures/clouds/posz.png",
        "resources/textures/clouds/negz.png"
    };

    auto skyboxTexture = cc::gfx::TextureCube::FromFiles(device.get(), cubemapFaces);

    auto skyboxSampler = cc::gfx::Sampler::Create(device.get(), cc::gfx::SamplerConfig{
        .minFilter = cc::gfx::TextureFilter::Linear,
        .magFilter = cc::gfx::TextureFilter::Linear,
        .wrapU = cc::gfx::TextureWrap::ClampToEdge,
        .wrapV = cc::gfx::TextureWrap::ClampToEdge,
        .wrapW = cc::gfx::TextureWrap::ClampToEdge
    });

    auto shadowShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/shadow.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/shadow.frag")
        .Build();

    auto pbrShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/lit.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/lit.frag")
        .Build();

    auto sphereShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/sphere_glass.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/sphere_glass.frag")
        .Build();

    auto waterShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/water.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/water.frag")
        .Build();

    auto skyboxShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex, "resources/shaders/skybox.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/skybox.frag")
        .Build();

    constexpr cc::u32 SHADOW_MAP_SIZE = 4096;
    auto shadowMapTexture = cc::gfx::Texture2D::Create(
        device.get(),
        SHADOW_MAP_SIZE,
        SHADOW_MAP_SIZE,
        cc::gfx::TextureFormat::Depth32F,
        nullptr
    );

    auto shadowFramebuffer = cc::gfx::Framebuffer::Create(device.get(), SHADOW_MAP_SIZE, SHADOW_MAP_SIZE)
        .AttachDepth(shadowMapTexture.get())
        .Build();

    auto shadowSampler = cc::gfx::Sampler::Create(device.get(), cc::gfx::SamplerConfig{
        .minFilter = cc::gfx::TextureFilter::Linear,
        .magFilter = cc::gfx::TextureFilter::Linear,
        .wrapU = cc::gfx::TextureWrap::ClampToBorder,
        .wrapV = cc::gfx::TextureWrap::ClampToBorder,
        .wrapW = cc::gfx::TextureWrap::ClampToBorder,
        .borderColor = {1.0f, 1.0f, 1.0f, 1.0f}
    });

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
        {{ 1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}
    };

    Vertex cubeVertices[] = {
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.8f, 0.2f, 0.2f}},
    };

    cc::u32 cubeIndices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    constexpr float planeSize = 15.0f;
    Vertex planeVertices[] = {
        {{-planeSize, 0.0f,  planeSize}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.3f, 0.35f}},
        {{ planeSize, 0.0f,  planeSize}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.3f, 0.35f}},
        {{ planeSize, 0.0f, -planeSize}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.3f, 0.35f}},
        {{-planeSize, 0.0f, -planeSize}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.3f, 0.35f}},
    };

    cc::u32 planeIndices[] = {0, 1, 2, 2, 3, 0};

    //NOTE: Water surface slightly above plane (pool water)
    Vertex waterVertices[] = {
        {{-planeSize, 0.02f,  planeSize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
        {{ planeSize, 0.02f,  planeSize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
        {{ planeSize, 0.02f, -planeSize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
        {{-planeSize, 0.02f, -planeSize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    };
    cc::u32 waterIndices[] = {0, 1, 2, 2, 3, 0};

    std::vector<Vertex> sphereVertices;
    std::vector<cc::u32> sphereIndices;
    generateSphere(sphereVertices, sphereIndices, 1.5f, 64, 64, cc::vec3f(0.9f, 0.9f, 1.0f));

    auto skyboxVertexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Vertex,
        sizeof(skyboxVertices), cc::gfx::BufferUsage::Static, skyboxVertices
    );
    auto cubeVertexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Vertex,
        sizeof(cubeVertices), cc::gfx::BufferUsage::Static, cubeVertices
    );
    auto cubeIndexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Index,
        sizeof(cubeIndices), cc::gfx::BufferUsage::Static, cubeIndices
    );
    auto planeVertexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Vertex,
        sizeof(planeVertices), cc::gfx::BufferUsage::Static, planeVertices
    );
    auto planeIndexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Index,
        sizeof(planeIndices), cc::gfx::BufferUsage::Static, planeIndices
    );
    auto waterVertexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Vertex,
        sizeof(waterVertices), cc::gfx::BufferUsage::Static, waterVertices
    );
    auto waterIndexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Index,
        sizeof(waterIndices), cc::gfx::BufferUsage::Static, waterIndices
    );
    auto sphereVertexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Vertex,
        sphereVertices.size() * sizeof(Vertex), cc::gfx::BufferUsage::Static, sphereVertices.data()
    );
    auto sphereIndexBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Index,
        sphereIndices.size() * sizeof(cc::u32), cc::gfx::BufferUsage::Static, sphereIndices.data()
    );

    auto shadowUniformBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Uniform,
        sizeof(ShadowUniforms), cc::gfx::BufferUsage::Dynamic, nullptr
    );
    auto pbrUniformBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Uniform,
        sizeof(PBRUniforms), cc::gfx::BufferUsage::Dynamic, nullptr
    );
    auto skyboxUniformBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Uniform,
        sizeof(SkyboxUniforms), cc::gfx::BufferUsage::Dynamic, nullptr
    );
    auto waterUniformBuffer = cc::gfx::Buffer::Create(
        device.get(), cc::gfx::BufferType::Uniform,
        sizeof(WaterUniforms), cc::gfx::BufferUsage::Dynamic, nullptr
    );

    auto vertexLayout = cc::gfx::VertexLayout::Create()
        .Binding(0, sizeof(Vertex), cc::gfx::VertexInputRate::PerVertex)
        .Attribute(0, 0, cc::gfx::VertexFormat::Float3, offsetof(Vertex, position))
        .Attribute(1, 0, cc::gfx::VertexFormat::Float3, offsetof(Vertex, normal))
        .Attribute(2, 0, cc::gfx::VertexFormat::Float3, offsetof(Vertex, color))
        .Build();

    auto skyboxVertexLayout = cc::gfx::VertexLayout::Create()
        .Binding(0, sizeof(SkyboxVertex), cc::gfx::VertexInputRate::PerVertex)
        .Attribute(0, 0, cc::gfx::VertexFormat::Float3, 0)
        .Build();

    auto shadowDescriptorLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Vertex)
        .Build();

    auto shadowDescriptorSet = cc::gfx::DescriptorSet::Create(device.get(), shadowDescriptorLayout.get())
        .Bind(0, shadowUniformBuffer.get())
        .Build();

    auto pbrDescriptorLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer,
            cc::gfx::ShaderStage::Vertex | cc::gfx::ShaderStage::Fragment)
        .Build();

    auto pbrDescriptorSet = cc::gfx::DescriptorSet::Create(device.get(), pbrDescriptorLayout.get())
        .Bind(0, pbrUniformBuffer.get())
        .Build();

    auto skyboxDescriptorLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Vertex)
        .Build();

    auto skyboxDescriptorSet = cc::gfx::DescriptorSet::Create(device.get(), skyboxDescriptorLayout.get())
        .Bind(0, skyboxUniformBuffer.get())
        .Build();

    auto waterDescriptorLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer,
                 cc::gfx::ShaderStage::Vertex | cc::gfx::ShaderStage::Fragment)
        .Build();

    auto waterDescriptorSet = cc::gfx::DescriptorSet::Create(device.get(), waterDescriptorLayout.get())
        .Bind(0, waterUniformBuffer.get())
        .Build();

    auto shadowPipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(shadowShader.get())
        .SetVertexLayout(vertexLayout.get())
        .AddDescriptorLayout(shadowDescriptorLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::Front)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(true)
        .SetDepthWrite(true)
        .SetDepthCompare(cc::gfx::CompareOp::Less)
        .Build();

    auto pbrPipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(pbrShader.get())
        .SetVertexLayout(vertexLayout.get())
        .AddDescriptorLayout(pbrDescriptorLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::Back)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(true)
        .SetDepthWrite(true)
        .SetDepthCompare(cc::gfx::CompareOp::Less)
        .SetBlendEnabled(false)
        .Build();

    auto spherePipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(sphereShader.get())
        .SetVertexLayout(vertexLayout.get())
        .AddDescriptorLayout(pbrDescriptorLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::Back)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(true)
        .SetDepthWrite(true)
        .SetDepthCompare(cc::gfx::CompareOp::Less)
        .SetBlendEnabled(true)
        .SetBlendSrcColor(cc::gfx::BlendFactor::SrcAlpha)
        .SetBlendDstColor(cc::gfx::BlendFactor::OneMinusSrcAlpha)
        .Build();

    auto waterPipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(waterShader.get())
        .SetVertexLayout(vertexLayout.get())
        .AddDescriptorLayout(waterDescriptorLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::Back)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(true)
        .SetDepthWrite(true)
        .SetDepthCompare(cc::gfx::CompareOp::Less)
        .SetBlendEnabled(true)
        .SetBlendSrcColor(cc::gfx::BlendFactor::SrcAlpha)
        .SetBlendDstColor(cc::gfx::BlendFactor::OneMinusSrcAlpha)
        .Build();

    auto skyboxPipeline = cc::gfx::Pipeline::Create(device.get())
        .SetShader(skyboxShader.get())
        .SetVertexLayout(skyboxVertexLayout.get())
        .AddDescriptorLayout(skyboxDescriptorLayout.get())
        .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
        .SetCullMode(cc::gfx::CullMode::None)
        .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
        .SetDepthTest(true)
        .SetDepthWrite(false)
        .SetDepthCompare(cc::gfx::CompareOp::LessEqual)
        .Build();

    shadowShader->SetUniformBlock("ShadowUniforms", 0);

    pbrShader->SetUniformBlock("PBRUniforms", 0);
    pbrShader->SetSampler("shadowMap", 0);
    pbrShader->SetSampler("environmentMap", 1);

    sphereShader->SetUniformBlock("PBRUniforms", 0);
    sphereShader->SetSampler("shadowMap", 0);
    sphereShader->SetSampler("environmentMap", 1);

    waterShader->SetUniformBlock("WaterUniforms", 0);
    waterShader->SetSampler("environmentMap", 0);

    skyboxShader->SetUniformBlock("SkyboxUniforms", 0);
    skyboxShader->SetSampler("skybox", 0);

    auto* pSkyboxVB = skyboxVertexBuffer.get();
    auto* pCubeVB   = cubeVertexBuffer.get();
    auto* pCubeIB   = cubeIndexBuffer.get();
    auto* pPlaneVB  = planeVertexBuffer.get();
    auto* pPlaneIB  = planeIndexBuffer.get();
    auto* pWaterVB  = waterVertexBuffer.get();
    auto* pWaterIB  = waterIndexBuffer.get();
    auto* pSphereVB = sphereVertexBuffer.get();
    auto* pSphereIB = sphereIndexBuffer.get();

    auto* pShadowPipeline  = shadowPipeline.get();
    auto* pPBRPipeline     = pbrPipeline.get();
    auto* pSpherePipeline  = spherePipeline.get();
    auto* pWaterPipeline   = waterPipeline.get();
    auto* pSkyboxPipeline  = skyboxPipeline.get();

    auto* pShadowDescSet   = shadowDescriptorSet.get();
    auto* pPBRDescSet      = pbrDescriptorSet.get();
    auto* pSkyboxDescSet   = skyboxDescriptorSet.get();
    auto* pWaterDescSet    = waterDescriptorSet.get();

    auto* pShadowUniformBuffer = shadowUniformBuffer.get();
    auto* pPBRUniformBuffer    = pbrUniformBuffer.get();
    auto* pSkyboxUniformBuffer = skyboxUniformBuffer.get();
    auto* pWaterUniformBuffer  = waterUniformBuffer.get();

    auto* pShadowFramebuffer = shadowFramebuffer.get();
    auto* pShadowMapTexture  = shadowMapTexture.get();
    auto* pShadowSampler     = shadowSampler.get();
    auto* pSkyboxTexture     = skyboxTexture.get();
    auto* pSkyboxSampler     = skyboxSampler.get();
    auto* pSwapchain         = swapchain.get();

    cc::u32 sphereIndexCount = static_cast<cc::u32>(sphereIndices.size());

    cc::mat4f cubeModel    = cc::translate(cc::vec3f(-3.0f, 0.5f, 0.0f));
    cc::mat4f sphereModel  = cc::translate(cc::vec3f(2.5f, 1.5f, 0.0f));
    cc::mat4f planeModel   = cc::mat4f::identity();
    cc::mat4f waterModel   = cc::mat4f::identity();

    ShadowUniforms  shadowUniforms{};
    PBRUniforms     pbrUniforms{};
    SkyboxUniforms  skyboxUniforms{};
    WaterUniforms   waterUniforms{};

    waterUniforms.waterColor   = cc::vec3f(0.0f, 0.4f, 0.6f);
    waterUniforms.waveStrength = 0.12f;

    cc::vec3f lightDir = cc::vec3f(0.5f, 1.0f, 0.3f).norm();
    float lightDistance  = 15.0f;
    float lightOrthoSize = 15.0f;

    auto renderGraph = cc::gfx::RenderGraph::Create(device.get());

    renderGraph->AddPass("ShadowPass")
        .SetFramebuffer(pShadowFramebuffer)
        .SetDepthClear(cc::gfx::ClearValue(1.0f, 0))
        .Execute([&](cc::gfx::CommandBuffer& cmd) {
            cmd.SetViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
            cmd.SetScissor(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
            cmd.BindPipeline(pShadowPipeline);
            cmd.BindDescriptorSet(0, pShadowDescSet);

            shadowUniforms.model = planeModel;
            pShadowUniformBuffer->Update(&shadowUniforms, sizeof(ShadowUniforms));
            cmd.BindVertexBuffer(0, pPlaneVB);
            cmd.BindIndexBuffer(pPlaneIB);
            cmd.DrawIndexed(6);

            shadowUniforms.model = cubeModel;
            pShadowUniformBuffer->Update(&shadowUniforms, sizeof(ShadowUniforms));
            cmd.BindVertexBuffer(0, pCubeVB);
            cmd.BindIndexBuffer(pCubeIB);
            cmd.DrawIndexed(36);

            shadowUniforms.model = sphereModel;
            pShadowUniformBuffer->Update(&shadowUniforms, sizeof(ShadowUniforms));
            cmd.BindVertexBuffer(0, pSphereVB);
            cmd.BindIndexBuffer(pSphereIB);
            cmd.DrawIndexed(sphereIndexCount);
        });

    renderGraph->AddPass("MainPass")
        .SetColorClear(cc::gfx::ClearValue(0.1f, 0.1f, 0.15f))
        .SetDepthClear(cc::gfx::ClearValue(1.0f, 0))
        .Read(pShadowMapTexture, cc::gfx::ResourceState::ShaderRead)
        .Write(cc::gfx::ResourceHandle::Backbuffer())
        .Execute([&](cc::gfx::CommandBuffer& cmd) {
            cmd.SetViewport(
                0.0f,
                0.0f,
                static_cast<float>(pSwapchain->GetWidth()),
                static_cast<float>(pSwapchain->GetHeight())
            );
            cmd.SetScissor(0, 0, pSwapchain->GetWidth(), pSwapchain->GetHeight());

            // Skybox
            pSkyboxTexture->Bind(0);
            pSkyboxSampler->Bind(0);
            cmd.BindPipeline(pSkyboxPipeline);
            cmd.BindDescriptorSet(0, pSkyboxDescSet);
            pSkyboxUniformBuffer->Update(&skyboxUniforms, sizeof(SkyboxUniforms));
            cmd.BindVertexBuffer(0, pSkyboxVB);
            cmd.Draw(36);

            // Common shadow/env bindings
            pShadowMapTexture->Bind(0);
            pShadowSampler->Bind(0);
            pSkyboxTexture->Bind(1);
            pSkyboxSampler->Bind(1);

            // Opaque PBR: plane
            cmd.BindPipeline(pPBRPipeline);
            cmd.BindDescriptorSet(0, pPBRDescSet);
            pbrUniforms.model     = planeModel;
            pbrUniforms.metallic  = 0.0f;
            pbrUniforms.roughness = 0.9f;
            pbrUniforms.ao        = 1.0f;
            pbrUniforms.isGlass   = 0.0f;
            pPBRUniformBuffer->Update(&pbrUniforms, sizeof(PBRUniforms));
            cmd.BindVertexBuffer(0, pPlaneVB);
            cmd.BindIndexBuffer(pPlaneIB);
            cmd.DrawIndexed(6);

            // Opaque PBR: cube
            pbrUniforms.model     = cubeModel;
            pbrUniforms.metallic  = 0.9f;
            pbrUniforms.roughness = 0.1f;
            pbrUniforms.ao        = 1.0f;
            pbrUniforms.isGlass   = 0.0f;
            pPBRUniformBuffer->Update(&pbrUniforms, sizeof(PBRUniforms));
            cmd.BindVertexBuffer(0, pCubeVB);
            cmd.BindIndexBuffer(pCubeIB);
            cmd.DrawIndexed(36);

            // Water (alpha-blended but drawn before glass sphere)
            pSkyboxTexture->Bind(0);
            pSkyboxSampler->Bind(0);
            cmd.BindPipeline(pWaterPipeline);
            cmd.BindDescriptorSet(0, pWaterDescSet);
            pWaterUniformBuffer->Update(&waterUniforms, sizeof(WaterUniforms));
            cmd.BindVertexBuffer(0, pWaterVB);
            cmd.BindIndexBuffer(pWaterIB);
            cmd.DrawIndexed(6);

            // Glass sphere
            pShadowMapTexture->Bind(0);
            pShadowSampler->Bind(0);
            pSkyboxTexture->Bind(1);
            pSkyboxSampler->Bind(1);
            cmd.BindPipeline(pSpherePipeline);
            cmd.BindDescriptorSet(0, pPBRDescSet);
            pbrUniforms.model     = sphereModel;
            pbrUniforms.metallic  = 0.0f;
            pbrUniforms.roughness = 0.05f;
            pbrUniforms.ao        = 1.0f;
            pbrUniforms.isGlass   = 1.0f;
            pPBRUniformBuffer->Update(&pbrUniforms, sizeof(PBRUniforms));
            cmd.BindVertexBuffer(0, pSphereVB);
            cmd.BindIndexBuffer(pSphereIB);
            cmd.DrawIndexed(sphereIndexCount);
        });

    renderGraph->SetSwapchain(swapchain.get());
    renderGraph->Compile();

    cc::log::Info("Initialization complete - PBR + Glass + Water + Skybox");
    cc::log::Info("Controls: LMB Drag = Orbit, Scroll = Zoom, R = Reset, Esc = Quit");

    float time = 0.0f;

    while (!window->ShouldClose()) {
        window->PollEvents();
        time += 0.016f;

        float lightAngle = time * 0.2f;
        lightDir = cc::vec3f(
            std::cos(lightAngle) * 0.5f,
            1.0f,
            std::sin(lightAngle) * 0.5f
        ).norm();

        cc::vec3f lightPos = lightDir * lightDistance;
        cc::mat4f lightView = cc::look_at(
            lightPos,
            cc::vec3f(0.0f, 0.0f, 0.0f),
            cc::vec3f(0.0f, 1.0f, 0.0f)
        );
        cc::mat4f lightProjection = cc::ortho(
            -lightOrthoSize, lightOrthoSize,
            -lightOrthoSize, lightOrthoSize,
            0.1f,
            lightDistance * 2.5f
        );
        cc::mat4f lightSpaceMatrix = lightProjection * lightView;

        shadowUniforms.lightSpaceMatrix = lightSpaceMatrix;

        float horizontalDist = g_camera.distance * std::cos(g_camera.pitch);
        float verticalDist   = g_camera.distance * std::sin(g_camera.pitch);
        cc::vec3f cameraPos = cc::vec3f(
            std::cos(g_camera.yaw) * horizontalDist,
            verticalDist,
            std::sin(g_camera.yaw) * horizontalDist
        );

        cc::vec3f targetPos = cc::vec3f(0.0f, 1.0f, 0.0f);
        cc::vec3f upDir     = cc::vec3f(0.0f, 1.0f, 0.0f);
        cc::f32 aspect = static_cast<cc::f32>(swapchain->GetWidth()) /
                         static_cast<cc::f32>(swapchain->GetHeight());

        cc::mat4f view = cc::look_at(cameraPos + targetPos, targetPos, upDir);
        cc::mat4f projection = cc::perspective(cc::rad(45.0f), aspect, 0.1f, 100.0f);

        pbrUniforms.view             = view;
        pbrUniforms.projection       = projection;
        pbrUniforms.lightSpaceMatrix = lightSpaceMatrix;
        pbrUniforms.lightDir         = lightDir;
        pbrUniforms.lightColor       = cc::vec3f(3.0f, 3.0f, 3.0f);
        pbrUniforms.viewPos          = cameraPos + targetPos;
        pbrUniforms.albedo           = cc::vec3f(1.0f, 1.0f, 1.0f);

        waterUniforms.model      = waterModel;
        waterUniforms.view       = view;
        waterUniforms.projection = projection;
        waterUniforms.viewPos    = cameraPos + targetPos;
        waterUniforms.time       = time;

        cc::mat4f viewNoTranslation = view;
        viewNoTranslation[3][0] = 0.0f;
        viewNoTranslation[3][1] = 0.0f;
        viewNoTranslation[3][2] = 0.0f;
        skyboxUniforms.viewProjection = projection * viewNoTranslation;

        renderGraph->Execute();
        swapchain->Present();
    }

    device->WaitIdle();
    cc::log::Info("Shutting down...");
    return 0;
}
