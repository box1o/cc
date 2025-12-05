#pragma once
#include "cc/gfx/gfx.hpp"

#include <atomic>

#include "Scene.hpp"
#include "cc/renderer/renderer.hpp"
#include "cc/renderer/camera/editor_camera.hpp"

namespace cc {



class Application final {
public:
    Application() ;
    ~Application() ;

    void Run();


private:
    std::atomic<bool> running_{true};

    scope<gfx::Window> window_;
    scope<gfx::Device> device_;
    scope<gfx::Swapchain> swapchain_;

    ref<gfx::events::EventBus> eventBus_;


    scope<Scene> scene_;
    scope<Renderer> renderer_;
    scope<EditorCamera> camera_;





};





}
