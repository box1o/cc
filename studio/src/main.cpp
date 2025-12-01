#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/gfx.hpp>
#include "GLFW/glfw3.h"
#include "camera/camera.hpp"
#include "camera/editor_camera.hpp"
#include "common/functions.hpp"
#include "skybox.hpp"






int main()
{
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

    cc::CameraProjection camProj{};
    camProj.type   = cc::CameraProjectionType::Perspective;
    camProj.yfov   = cc::radians(45.0f);
    camProj.aspect = window->GetAspectRatio();
    camProj.znear  = 0.1f;
    camProj.zfar   = 100.0f;

    cc::EditorCameraConfig camCfg{};
    camCfg.pivot    = {0.0f, 0.0f, 0.0f};
    camCfg.distance = 5.0f;
    camCfg.yaw      = 0.0f;
    camCfg.pitch    = 0.3f;

    cc::EditorCamera editorCamera = cc::EditorCamera::Builder{}
        .SetName("StudioCamera")
        .SetProjection(camProj)
        .SetConfig(camCfg)
        .SetInitialPivot(camCfg.pivot)
        .SetEventBus(eBus)
        .SetViewportSize(window->GetWidth(), window->GetHeight())
        .Build();

    editorCamera.Focus({0.0f, 0.0f, 0.0f}, 1.0f);



    auto graph = cc::gfx::RenderGraph::Create(device.get());
    graph->SetSwapchain(swapchain.get());



    Skybox skybox;
    skybox.Initialize(device.get());
    skybox.Resize(swapchain.get());
    auto& skyboxPass = graph->AddPass("Skybox", cc::gfx::PassType::Graphics);
    skyboxPass
        .WriteBackbuffer(cc::gfx::ResourceState::ColorWrite)
        .SetClearColor(0.1f, 0.1f, 0.1f, 1.0f)
        .SetClearDepth(1.0f, 0)
        .Execute([&](cc::gfx::CommandBuffer& cmd) {
            skybox.Record(cmd, swapchain.get());
        });

    //NOTE: add more passes here for other models
    // auto& modelPass = graph->AddPass("Models", cc::gfx::PassType::Graphics);
    // modelPass.Execute([&](cc::gfx::CommandBuffer& cmd) { ... });

    graph->Compile();

    window->SetResizeCallback([&](cc::u32 width, cc::u32 height) {
        (void)width;
        (void)height;
        skybox.Resize(swapchain.get());

    });

    while (!window->ShouldClose())
    {
        skybox.Update(swapchain.get() , editorCamera.GetCamera().GetView());
        cc::f32 dt = (1/glfwGetTime());
        editorCamera.Tick(dt);

        //NOTE: update other systems / models here

        graph->Execute();
        swapchain->Present();
        window->PollEvents();
    }

    return 0;
}
