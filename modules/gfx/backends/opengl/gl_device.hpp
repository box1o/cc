#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/gfx/types.hpp>

struct GLFWwindow;

namespace cc::gfx {

class OpenGLDevice final : public Device {
public:
    ~OpenGLDevice() override;

    const DeviceCapabilities& GetCapabilities() const override { return capabilities_; }
    const DeviceInfo& GetInfo() const override { return info_; }
    Backend GetBackend() const override { return Backend::OpenGL; }

    void WaitIdle() override;

    scope<Buffer> CreateBuffer(BufferType type, u64 size, BufferUsage usage, const void* data) override;
    scope<Texture2D> CreateTexture2D(u32 width, u32 height, TextureFormat format, const void* data) override;
    scope<TextureCube> CreateTextureCube(u32 size, TextureFormat format) override;
    scope<Sampler> CreateSampler(const SamplerConfig& config) override;
    scope<Framebuffer> CreateDefaultFramebuffer(u32 width, u32 height) override;
    scope<DescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<DescriptorBinding>& bindings) override;
    scope<DescriptorSet> CreateDescriptorSet(
        DescriptorSetLayout* layout,
        const std::vector<BufferBinding>& bufferBindings,
        const std::vector<TextureBinding>& textureBindings
    ) override;
    scope<Pipeline> CreatePipeline(
        Shader* shader,
        VertexLayout* vertexLayout,
        const std::vector<DescriptorSetLayout*>& descriptorLayouts,
        PrimitiveTopology topology,
        const RasterizerState& rasterizer,
        const DepthStencilState& depthStencil,
        const BlendState& blend
    ) override;
    scope<CommandBuffer> CreateCommandBuffer() override;

    static scope<Device> CreateFromBuilder(const Device::Builder& builder);

    static void DebugCallbackImpl(
        unsigned int source,
        unsigned int type,
        unsigned int id,
        unsigned int severity,
        int length,
        const char* message,
        const void* userParam
    );

private:
    OpenGLDevice() = default;

    void Initialize(GLFWwindow* window, bool enableValidation);
    void QueryCapabilities();
    void QueryInfo();
    static void SetupDebugCallback();

    DeviceCapabilities capabilities_{};
    DeviceInfo info_{};
};

scope<Device> CreateOpenGLDevice(const Device::Builder& builder);

} // namespace cc::gfx
