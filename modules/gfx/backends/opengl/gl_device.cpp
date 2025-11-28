#include "gl_device.hpp"
#include <cc/gfx/window/window.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace cc::gfx {

constexpr unsigned int IGNORED_MESSAGE_IDS[] = {131169, 131185, 131218, 131204};

static GLFWwindow* GetGLFWWindowHandle(Window* window) noexcept {
    return static_cast<GLFWwindow*>(window->GetNativeHandle());
}

void APIENTRY GLDebugCallback(
    unsigned int source,
    unsigned int type,
    unsigned int messageId,
    unsigned int severity,
    int length,
    const char* message,
    const void* userParam
) {
    OpenGLDevice::DebugCallbackImpl(source, type, messageId, severity, length, message, userParam);
}

void OpenGLDevice::DebugCallbackImpl(
    unsigned int source,
    unsigned int type,
    unsigned int messageId,
    unsigned int severity,
    int /*length*/,
    const char* message,
    const void* /*userParam*/
) {
    for (unsigned int ignored : IGNORED_MESSAGE_IDS) {
        if (messageId == ignored) {
            return;
        }
    }

    const char* sourceStr = "Unknown";
    switch (source) {
        case GL_DEBUG_SOURCE_API:           sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:   sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:   sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:         sourceStr = "Other"; break;
    }

    const char* typeStr = "Unknown";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            log::Error("[OpenGL] {} - {}: {}", sourceStr, typeStr, message);
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            log::Warn("[OpenGL] {} - {}: {}", sourceStr, typeStr, message);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            log::Info("[OpenGL] {} - {}: {}", sourceStr, typeStr, message);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            log::Trace("[OpenGL] {} - {}: {}", sourceStr, typeStr, message);
            break;
    }
}

void OpenGLDevice::SetupDebugCallback() {
    int flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        log::Info("OpenGL debug output enabled");
    } else {
        log::Warn("OpenGL debug context not available");
    }
}

void OpenGLDevice::Initialize(GLFWwindow* window, bool enableValidation) {
    if (window == nullptr) {
        throw std::runtime_error("OpenGLDevice::Initialize: window is null");
    }

    glfwMakeContextCurrent(window);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    log::Info("GLAD initialized successfully");

    if (enableValidation) {
        SetupDebugCallback();
    }

    QueryCapabilities();
    QueryInfo();

    log::Info("OpenGL Device initialized");
    log::Info("  Vendor: {}", info_.vendorName != nullptr ? info_.vendorName : "Unknown");
    log::Info("  Renderer: {}", info_.rendererName != nullptr ? info_.rendererName : "Unknown");
    log::Info("  Version: {}", info_.apiVersion != nullptr ? info_.apiVersion : "Unknown");
    log::Info("  GLSL: {}", info_.shadingLanguageVersion != nullptr ? info_.shadingLanguageVersion : "Unknown");
}

void OpenGLDevice::QueryCapabilities() {
    int value = 0;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    capabilities_.maxTextureSize = static_cast<u32>(value);

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value);
    capabilities_.maxTextureUnits = static_cast<u32>(value);

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
    capabilities_.maxVertexAttributes = static_cast<u32>(value);

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
    capabilities_.maxUniformBufferSize = static_cast<u32>(value);

    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &value);
    capabilities_.maxStorageBufferSize = static_cast<u32>(value);

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
    capabilities_.maxColorAttachments = static_cast<u32>(value);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &value);
    capabilities_.maxComputeWorkGroupSize[0] = static_cast<u32>(value);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &value);
    capabilities_.maxComputeWorkGroupSize[1] = static_cast<u32>(value);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &value);
    capabilities_.maxComputeWorkGroupSize[2] = static_cast<u32>(value);

    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &value);
    capabilities_.maxComputeWorkGroupInvocations = static_cast<u32>(value);

    float fvalue = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &fvalue);
    capabilities_.maxAnisotropy = fvalue;

    capabilities_.supportsCompute = (GLAD_GL_VERSION_4_3 != 0);
    capabilities_.supportsGeometryShader = (GLAD_GL_VERSION_3_2 != 0);
    capabilities_.supportsTessellation = (GLAD_GL_VERSION_4_0 != 0);
    capabilities_.supportsBindlessTextures = (GLAD_GL_ARB_bindless_texture != 0);
    capabilities_.supportsRayTracing = false;
}

void OpenGLDevice::QueryInfo() {
    info_.vendorName = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    info_.rendererName = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    info_.apiVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    info_.shadingLanguageVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
}

OpenGLDevice::~OpenGLDevice() {
    log::Info("OpenGL device destroyed");
}

void OpenGLDevice::WaitIdle() {
    glFinish();
}

scope<Device> OpenGLDevice::CreateFromBuilder(const Device::Builder& builder) {
    auto device = scope<OpenGLDevice>(new OpenGLDevice());

    GLFWwindow* windowHandle = GetGLFWWindowHandle(builder.window_);
    device->Initialize(windowHandle, builder.enableValidation_);

    return device;
}

scope<Device> CreateOpenGLDevice(const Device::Builder& builder) {
    return OpenGLDevice::CreateFromBuilder(builder);
}

} // namespace cc::gfx
