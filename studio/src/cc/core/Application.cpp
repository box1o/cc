#include "Application.hpp"
#include "types.hpp"

namespace cc{
Application::Application() 
:running_(true) {
    log::Init("studio");


    window_ = gfx::Window::Create()
        .SetBackend(gfx::WindowBackend::GLFW)
        .SetTitle("studio")
        .SetSize(1280, 720)
        .Build();

    eventBus_ = gfx::events::EventBus::Create();
    window_->SetEventBus(eventBus_);

    device_ = gfx::Device::Create(window_.get())
        .SetBackend(cc::gfx::Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    swapchain_ = gfx::Swapchain::Create(window_.get(), device_.get());
    scene_ = createScope<Scene>("main");
    // scene->AddComponent<Renderable>(entity);


    // renderer_ = createScope<Renderer>();
    // CameraProjection camProj{};
    // camProj.type   = CameraProjectionType::Perspective;
    // camProj.yfov   = radians(60.0f);
    // camProj.aspect = window_->GetAspectRatio();
    // camProj.znear  = 0.1f;
    // camProj.zfar   = 100.0f;
    //
    // EditorCameraConfig camCfg{};
    // camCfg.pivot    = {0.0f, 0.0f, 0.0f};
    // camCfg.distance = 5.0f;
    // camCfg.yaw      = 0.0f;
    // camCfg.pitch    = 0.3f;

    camera_ = EditorCamera::Builder{}
        .SetName("StudioCamera")
        // .SetProjection(camProj)
        // .SetConfig(camCfg)
        // .SetInitialPivot(camCfg.pivot)
        .SetEventBus(eventBus_)
        .SetViewportSize(window_->GetWidth(), window_->GetHeight())
        .Build();


    // camera_.Focus({0.0f, 0.0f, 0.0f}, 1.0f);


}
Application::~Application(){
    swapchain_.reset();
    device_.reset();
    window_.reset();
}




void Application::Run(){

    while (running_ && !window_->ShouldClose()) {
        window_->PollEvents();





        //Plan 
        // renderer_->BeginScene(camera_);
        // renderer_->EndScene();

        //     for (auto&& [entity, renderable] : scene->View<Renderable>()) {
        //     }





        swapchain_->Present();

    }

}


}
