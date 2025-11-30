#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/gfx.hpp>
#include <cc/gfx/camera/camera.hpp>
#include <cc/gfx/camera/editor_camera.hpp>
#include <cc/gfx/events/bus.hpp>
#include <cc/gfx/model/loader.hpp>
#include <GLFW/glfw3.h>
#include <array>
#include <filesystem>

struct SkyboxVertex {
    cc::vec3f position;
};

struct SkyboxUniforms {
    cc::mat4f viewProjection;
};

struct ModelUniforms {
    cc::mat4f model;
    cc::mat4f view;
    cc::mat4f projection;
    cc::vec3f cameraPos;
    float _pad0;
};

struct DirectionalLightGPU {
    cc::vec3f direction;
    float _pad0;
    cc::vec3f color;
    float intensity;
};

struct SceneLightingUniforms {
    DirectionalLightGPU dirLight;
    cc::vec3f ambientColor;
    float ambientIntensity;
};

struct MaterialParamsGPU {
    cc::vec4f baseColorFactor;
    cc::vec3f emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    int   hasBaseColorTex;
    int   hasMetallicRoughnessTex;
    int   hasNormalTex;
    int   _pad;
};

struct DrawPrimitive {
    cc::mat4f                        worldMatrix;
    const cc::gfx::MeshPrimitive*    primitive{nullptr};
    cc::gfx::NodeID                  nodeId{cc::gfx::INVALID_NODE_ID};
};

static cc::mat4f ComputeLocalMatrix(const cc::gfx::Node& node) {
    if (node.hasLocalMatrix) return node.localMatrix;
    const auto& t = node.local;
    cc::mat4f T = cc::translate(t.translation);
    cc::mat4f R = t.rotation.to_mat4();
    cc::mat4f S = cc::scale(t.scale);
    return T * R * S;
}

static void CollectDrawPrimitivesRecursive(
    const cc::gfx::Scene& scene,
    cc::gfx::NodeID nodeId,
    const cc::mat4f& parentWorld,
    std::vector<DrawPrimitive>& out
) {
    const cc::gfx::Node* node = scene.GetNode(nodeId);
    if (!node) return;

    cc::mat4f local = ComputeLocalMatrix(*node);
    cc::mat4f world = parentWorld * local;

    if (node->mesh) {
        const cc::gfx::Mesh* mesh = scene.GetMesh(*node->mesh);
        if (mesh) {
            for (const auto& prim : mesh->primitives) {
                if (prim.vertexBuffer && prim.indexBuffer) {
                    DrawPrimitive dp{};
                    dp.worldMatrix = world;
                    dp.primitive   = &prim;
                    dp.nodeId      = nodeId;
                    out.push_back(dp);
                }
            }
        }
    }

    for (cc::gfx::NodeID childId : node->children) {
        CollectDrawPrimitivesRecursive(scene, childId, world, out);
    }
}

int main() {
    cc::log::Init("studio");

    auto window = cc::gfx::Window::Create()
        .SetTitle("Studio")
        .SetSize(1280, 720)
        .SetVSync(true)
        .Build();

    auto device = cc::gfx::Device::Create(window.get(), cc::gfx::Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    auto swapchain = cc::gfx::Swapchain::Create(window.get(), device.get());

    auto eventBus = cc::gfx::events::EventBus::Create();
    window->SetEventBus(eventBus);

    cc::gfx::CameraProjection camProj{};
    camProj.type   = cc::gfx::CameraProjectionType::Perspective;
    camProj.yfov   = cc::rad(45.0f);
    camProj.aspect = window->GetAspectRatio();
    camProj.znear  = 0.1f;
    camProj.zfar   = 100.0f;

    cc::gfx::EditorCameraConfig camCfg{};
    camCfg.pivot    = {0.0f, 0.0f, 0.0f};
    camCfg.distance = 5.0f;
    camCfg.yaw      = 0.0f;
    camCfg.pitch    = 0.3f;

    cc::gfx::EditorCamera editorCamera = cc::gfx::EditorCamera::Builder{}
        .SetName("StudioCamera")
        .SetProjection(camProj)
        .SetConfig(camCfg)
        .SetInitialPivot(camCfg.pivot)
        .SetEventBus(eventBus)
        .SetViewportSize(window->GetWidth(), window->GetHeight())
        .Build();

    editorCamera.Focus({0.0f, 0.0f, 0.0f}, 1.0f);

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

    auto skyboxVB = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Vertex,
        sizeof(skyboxVertices),
        cc::gfx::BufferUsage::Static,
        skyboxVertices
    );

    auto skyboxLayout = cc::gfx::VertexLayout::Create()
        .Binding(0, sizeof(SkyboxVertex), cc::gfx::VertexInputRate::PerVertex)
        .Attribute(0, 0, cc::gfx::VertexFormat::Float3, 0)
        .Build();

    auto skyboxShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex,   "resources/shaders/skybox.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/skybox.frag")
        .Build();

    auto modelShader = cc::gfx::Shader::Create(device.get())
        .AddStage(cc::gfx::ShaderStage::Vertex,   "resources/shaders/model.vert")
        .AddStage(cc::gfx::ShaderStage::Fragment, "resources/shaders/model.frag")
        .Build();

    auto skyboxUBO = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Uniform,
        sizeof(SkyboxUniforms),
        cc::gfx::BufferUsage::Dynamic
    );

    auto modelUBO = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Uniform,
        sizeof(ModelUniforms),
        cc::gfx::BufferUsage::Dynamic
    );

    auto lightingUBO = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Uniform,
        sizeof(SceneLightingUniforms),
        cc::gfx::BufferUsage::Dynamic
    );

    auto materialUBO = cc::gfx::Buffer::Create(
        device.get(),
        cc::gfx::BufferType::Uniform,
        sizeof(MaterialParamsGPU),
        cc::gfx::BufferUsage::Dynamic
    );

    auto skyboxDescLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Vertex)
        .Build();

    auto modelDescLayout = cc::gfx::DescriptorSetLayout::Create(device.get())
        .Binding(0, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Vertex)
        .Binding(1, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Fragment)
        .Binding(2, cc::gfx::DescriptorType::UniformBuffer, cc::gfx::ShaderStage::Fragment)
        .Build();

    auto skyboxDescSet = cc::gfx::DescriptorSet::Create(device.get(), skyboxDescLayout.get())
        .Bind(0, skyboxUBO.get())
        .Build();

    auto modelDescSet = cc::gfx::DescriptorSet::Create(device.get(), modelDescLayout.get())
        .Bind(0, modelUBO.get())
        .Bind(1, lightingUBO.get())
        .Bind(2, materialUBO.get())
        .Build();

    skyboxShader->Bind();
    skyboxShader->SetUniformBlock("SkyboxUniforms", 0);
    skyboxShader->SetSampler("skybox", 0);
    skyboxShader->Unbind();

    modelShader->Bind();
    modelShader->SetUniformBlock("ModelUniforms", 0);
    modelShader->SetUniformBlock("SceneLighting", 1);
    modelShader->SetUniformBlock("MaterialParams", 2);
    modelShader->SetSampler("uBaseColorTex",         0); // texture unit 0
    modelShader->SetSampler("uMetallicRoughnessTex", 1); // texture unit 1
    modelShader->SetSampler("uNormalTex",            2); // texture unit 2
    modelShader->Unbind();

    auto cmd = cc::gfx::CommandBuffer::Create(device.get());

    auto resourceLoader = cc::gfx::ResourceLoader::Create(device.get());
    cc::gfx::MeshLoadOptions loadOpts{};
    loadOpts.mode  = cc::gfx::LoadMode::Sync;
    loadOpts.flags = cc::gfx::MeshLoadFlag::FlipTexcoordY;

    auto meshLoader = cc::gfx::MeshLoader::FromFile(
        resourceLoader.get(),
        std::filesystem::path("resources/models/pool.glb"),
        loadOpts
    );

    const cc::gfx::Scene* loadedScene = nullptr;
    std::vector<DrawPrimitive> drawPrims;

    meshLoader->Wait();
    if (meshLoader->HasError()) {
        cc::log::Error("Failed to load model: {}", meshLoader->GetErrorMessage());
    } else {
        loadedScene = &meshLoader->GetScene();
        const auto& scene = *loadedScene;

        const auto& roots = scene.GetRootNodes();
        drawPrims.clear();
        for (cc::gfx::NodeID rootId : roots) {
            CollectDrawPrimitivesRecursive(scene, rootId, cc::mat4f::identity(), drawPrims);
        }
        cc::log::Info("Collected {} draw primitives from scene", drawPrims.size());
    }

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

    std::unique_ptr<cc::gfx::Pipeline> modelPipeline;
    if (!drawPrims.empty() && drawPrims.front().primitive && drawPrims.front().primitive->vertexLayout) {
        modelPipeline = cc::gfx::Pipeline::Create(device.get())
            .SetShader(modelShader.get())
            .SetVertexLayout(drawPrims.front().primitive->vertexLayout)
            .AddDescriptorLayout(modelDescLayout.get())
            .SetPrimitiveTopology(cc::gfx::PrimitiveTopology::TriangleList)
            .SetCullMode(cc::gfx::CullMode::None)
            .SetFrontFace(cc::gfx::FrontFace::CounterClockwise)
            .SetDepthTest(true)
            .SetDepthWrite(true)
            .SetDepthCompare(cc::gfx::CompareOp::Less)
            .Build();
    } else {
        cc::log::Warn("Model pipeline not created: no primitives available");
    }

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

    {
        SceneLightingUniforms lighting{};
        lighting.dirLight.direction = cc::vec3f{-0.5f, -1.0f, -0.3f};
        lighting.dirLight.color     = cc::vec3f{1.0f, 1.0f, 1.0f};
        lighting.dirLight.intensity = 1.0f;

        lighting.ambientColor     = cc::vec3f{0.2f, 0.2f, 0.25f};
        lighting.ambientIntensity = 1.0f;

        lightingUBO->Update(&lighting, sizeof(SceneLightingUniforms));
    }

    while (!window->ShouldClose()) {
        window->PollEvents();

        cc::f32 dt = 0.016f;
        editorCamera.Tick(dt);

        const auto w = swapchain->GetWidth();
        const auto h = swapchain->GetHeight();
        if (h > 0 && editorCamera.GetCamera().GetProjection().type == cc::gfx::CameraProjectionType::Perspective) {
            editorCamera.SetViewportSize(w, h);
        }

        const auto& cam = editorCamera.GetCamera();
        const cc::mat4f& view    = cam.GetView();
        const cc::mat4f& projMat = cam.GetProjectionMatrix();
        const cc::vec3f& camPos  = cam.GetPosition();

        cc::mat4f skyView = view;
        skyView[3][0] = 0.0f;
        skyView[3][1] = 0.0f;
        skyView[3][2] = 0.0f;

        SkyboxUniforms skyU{};
        skyU.viewProjection = projMat * skyView;
        skyboxUBO->Update(&skyU, sizeof(SkyboxUniforms));

        cmd->Begin();
        cmd->BeginRenderPass(
            swapchain->GetFramebuffer(),
            cc::gfx::ClearValue::Color(0.1f, 0.1f, 0.15f, 1.0f),
            cc::gfx::ClearValue::Depth(1.0f, 0)
        );

        cmd->SetViewport(0.0f, 0.0f, static_cast<cc::f32>(w), static_cast<cc::f32>(h));
        cmd->SetScissor(0, 0, w, h);

        skyboxTex->Bind(0);
        skyboxSampler->Bind(0);

        cmd->BindPipeline(skyboxPipeline.get());
        cmd->BindDescriptorSet(0, skyboxDescSet.get());
        cmd->BindVertexBuffer(0, skyboxVB.get());
        cmd->Draw(36);

        if (modelPipeline && loadedScene && !drawPrims.empty()) {
            const auto& scene     = *loadedScene;
            const auto& materials = scene.GetMaterials();

            cmd->BindPipeline(modelPipeline.get());
            cmd->BindDescriptorSet(0, modelDescSet.get());

            for (const auto& dp : drawPrims) {
                if (!dp.primitive) continue;

                ModelUniforms mu{};
                mu.model      = dp.worldMatrix;
                mu.view       = view;
                mu.projection = projMat;
                mu.cameraPos  = camPos;
                modelUBO->Update(&mu, sizeof(ModelUniforms));

                MaterialParamsGPU mp{};
                mp.baseColorFactor   = cc::vec4f{1.0f, 1.0f, 1.0f, 1.0f};
                mp.emissiveFactor    = cc::vec3f{0.0f, 0.0f, 0.0f};
                mp.metallicFactor    = 1.0f;
                mp.roughnessFactor   = 1.0f;
                mp.hasBaseColorTex   = 0;
                mp.hasMetallicRoughnessTex = 0;
                mp.hasNormalTex      = 0;

                const cc::gfx::Material* mat = nullptr;
                if (dp.primitive->material) {
                    mat = scene.GetMaterial(*dp.primitive->material);
                }
                if (mat) {
                    mp.baseColorFactor = mat->baseColorFactor;
                    mp.emissiveFactor  = mat->emissiveFactor;
                    mp.metallicFactor  = mat->metallicFactor;
                    mp.roughnessFactor = mat->roughnessFactor;

                    if (mat->baseColorTexture.texture)           mp.hasBaseColorTex = 1;
                    if (mat->metallicRoughnessTexture.texture)   mp.hasMetallicRoughnessTex = 1;
                    if (mat->normalTexture.texture)              mp.hasNormalTex = 1;
                }

                materialUBO->Update(&mp, sizeof(MaterialParamsGPU));

                // Bind material textures to fixed units
                if (mat && mat->baseColorTexture.texture) {
                    mat->baseColorTexture.texture->Bind(0);
                    if (mat->baseColorTexture.sampler) mat->baseColorTexture.sampler->Bind(0);
                }
                if (mat && mat->metallicRoughnessTexture.texture) {
                    mat->metallicRoughnessTexture.texture->Bind(1);
                    if (mat->metallicRoughnessTexture.sampler) mat->metallicRoughnessTexture.sampler->Bind(1);
                }
                if (mat && mat->normalTexture.texture) {
                    mat->normalTexture.texture->Bind(2);
                    if (mat->normalTexture.sampler) mat->normalTexture.sampler->Bind(2);
                }

                cmd->BindVertexBuffer(0, dp.primitive->vertexBuffer.get());
                cmd->BindIndexBuffer(dp.primitive->indexBuffer.get(), dp.primitive->indexType);
                cmd->DrawIndexed(
                    dp.primitive->indexCount,
                    1,
                    dp.primitive->firstIndex,
                    dp.primitive->vertexOffset,
                    0
                );
            }
        }

        cmd->EndRenderPass();
        cmd->End();
        cmd->Submit();

        swapchain->Present();
    }

    device->WaitIdle();
    return 0;
}
