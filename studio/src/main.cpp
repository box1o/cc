#include "window/window.hpp"
#include <cc/core/core.hpp>
#include <cc/gfx/gfx.hpp>
#include <glad/glad.h>
#include "GLFW/glfw3.h"

int main(){
    cc::log::Init("studio");
    cc::log::Info("Starting ....");

    auto window = cc::gfx::Window::Create()
        .SetTitle("studio")
        .SetSize(1280, 720)
        .Build();

    auto* glfw_window = static_cast<GLFWwindow*>(window->GetNativeHandle());
    glfwMakeContextCurrent(glfw_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cc::log::Critical("Failed to initialize GLAD");
        return -1;
    }


    glViewport(0, 0, window->GetWidth(), window->GetHeight());

    window->SetVSync(window->IsVSync());

    while (!window->ShouldClose()){
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glfwSwapBuffers(glfw_window);
        window->PollEvents();
    }

    return 0;
}
