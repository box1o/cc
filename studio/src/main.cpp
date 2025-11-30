#include <cc/gfx/gfx.hpp>


int main (){

    auto window = cc::gfx::Window::Create()
        .SetTitle("studio")
        .SetBackend(cc::gfx::WindowBackend::GLFW)
        .Build();


    auto device = cc::gfx::Device::Create(window.get(), cc::gfx::Backend::OpenGL)
        .EnableValidation(true)
        .Build();

    auto swapchain = cc::gfx::Swapchain::Create(window.get(), device.get());


    while (!window->ShouldClose()) {
        swapchain->Clear({0.1f, 0.1f, 0.1f, 1.0f});

        swapchain->Present();
        window->SwapBuffers();
        window->PollEvents();
    }



    return 0;
}
