#pragma once

namespace cc::gfx {

//NOTE: Core Classes
class Device;
class Window;
class Buffer;
class Texture;
class Texture2D;
class TextureCube;
class Sampler;
class Framebuffer;
class Shader;
class Pipeline;
class Swapchain;
class VertexLayout;
class DescriptorSetLayout;
class DescriptorSet;
class CommandBuffer;

//NOTE: Render Graph Classes
class RenderGraph;
class RenderPass;
class PassBuilder;
struct ResourceHandle;

//NOTE: Implementation Classes
class BufferImpl;
class TextureImpl;
class SamplerImpl;
class FramebufferImpl;
class ShaderImpl;
class PipelineImpl;
class DescriptorSetLayoutImpl;
class DescriptorSetImpl;
class CommandBufferImpl;

//NOTE: Configuration Structs
struct RenderPassBeginInfo;
struct ColorAttachmentInfo;
struct DepthAttachmentInfo;

} // namespace cc::gfx
