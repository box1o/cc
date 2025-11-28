#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <vector>

namespace cc::gfx {

struct BufferBinding;
struct TextureBinding;

class Device {
public:
    class Builder {
    public:
        Builder& SetBackend(Backend backend);
        Builder& EnableValidation(bool enable);
        scope<Device> Build();

    private:
        Backend backend_{Backend::OpenGL};
        bool enableValidation_{false};
        Window* window_{nullptr};

        friend class Device;
        friend class OpenGLDevice;
    };

    virtual ~Device() = default;

    static Builder Create(Window* window, Backend backend = Backend::OpenGL);

    //NOTE: Device Info
    virtual const DeviceCapabilities& GetCapabilities() const = 0;
    virtual const DeviceInfo& GetInfo() const = 0;
    virtual Backend GetBackend() const = 0;

    //NOTE: Synchronization
    virtual void WaitIdle() = 0;

    //NOTE: Resource Creation
    virtual scope<Buffer> CreateBuffer(BufferType type, u64 size, BufferUsage usage = BufferUsage::Static, const void* data = nullptr) = 0;
    virtual scope<Texture2D> CreateTexture2D(u32 width, u32 height, TextureFormat format = TextureFormat::RGBA8, const void* data = nullptr) = 0;
    virtual scope<TextureCube> CreateTextureCube(u32 size, TextureFormat format = TextureFormat::RGBA8) = 0;
    virtual scope<Sampler> CreateSampler(const SamplerConfig& config = {}) = 0;
    virtual scope<Framebuffer> CreateDefaultFramebuffer(u32 width, u32 height) = 0;

    //NOTE: Descriptor Resources
    virtual scope<DescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<DescriptorBinding>& bindings) = 0;
    virtual scope<DescriptorSet> CreateDescriptorSet(
        DescriptorSetLayout* layout,
        const std::vector<BufferBinding>& bufferBindings,
        const std::vector<TextureBinding>& textureBindings
    ) = 0;

    //NOTE: Pipeline
    virtual scope<Pipeline> CreatePipeline(
        Shader* shader,
        VertexLayout* vertexLayout,
        const std::vector<DescriptorSetLayout*>& descriptorLayouts,
        PrimitiveTopology topology,
        const RasterizerState& rasterizer,
        const DepthStencilState& depthStencil,
        const BlendState& blend
    ) = 0;

    //NOTE: Command Buffer
    virtual scope<CommandBuffer> CreateCommandBuffer() = 0;

protected:
    Device() = default;
};

} // namespace cc::gfx
