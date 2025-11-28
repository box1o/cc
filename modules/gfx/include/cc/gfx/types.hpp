#pragma once
#include <cc/core/types.hpp>

namespace cc::gfx {

// NOTE: Backend Types
enum class Backend : u8 {
    OpenGL = 0,
    Vulkan = 1,
    Metal  = 2
};

enum class WindowBackend : u8 {
    GLFW   = 0,
    Native = 1
};

// NOTE: Buffer Types
enum class BufferType : u8 {
    Vertex  = 0,
    Index   = 1,
    Uniform = 2,
    Storage = 3,
    Staging = 4
};

enum class BufferUsage : u8 {
    Static  = 0,
    Dynamic = 1,
    Stream  = 2
};

enum class IndexType : u8 {
    U16 = 0,
    U32 = 1
};

// NOTE: Texture Types
enum class TextureFormat : u8 {
    R8,
    RG8,
    RGB8,
    RGBA8,
    SRGB8,
    SRGB8_A8,
    R16F,
    RG16F,
    RGB16F,
    RGBA16F,
    R32F,
    RG32F,
    RGB32F,
    RGBA32F,
    Depth16,
    Depth24,
    Depth32F,
    Depth24Stencil8,
    Depth32FStencil8
};

enum class TextureType : u8 {
    Texture2D      = 0,
    Texture3D      = 1,
    TextureCube    = 2,
    Texture2DArray = 3
};

enum class TextureUsage : u8 {
    Sampled      = 1 << 0,
    Storage      = 1 << 1,
    RenderTarget = 1 << 2,
    DepthStencil = 1 << 3
};

constexpr TextureUsage operator|(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<u8>(a) | static_cast<u8>(b));
}

constexpr bool operator&(TextureUsage a, TextureUsage b) {
    return (static_cast<u8>(a) & static_cast<u8>(b)) != 0;
}

enum class TextureFilter : u8 {
    Nearest = 0,
    Linear  = 1
};

enum class TextureWrap : u8 {
    Repeat        = 0,
    MirrorRepeat  = 1,
    ClampToEdge   = 2,
    ClampToBorder = 3
};

// NOTE: Pipeline Types
enum class PrimitiveTopology : u8 {
    PointList     = 0,
    LineList      = 1,
    LineStrip     = 2,
    TriangleList  = 3,
    TriangleStrip = 4
};

enum class CullMode : u8 {
    None  = 0,
    Front = 1,
    Back  = 2
};

enum class FrontFace : u8 {
    CounterClockwise = 0,
    Clockwise        = 1
};

enum class CompareOp : u8 {
    Never        = 0,
    Less         = 1,
    Equal        = 2,
    LessEqual    = 3,
    Greater      = 4,
    NotEqual     = 5,
    GreaterEqual = 6,
    Always       = 7
};

enum class BlendFactor : u8 {
    Zero             = 0,
    One              = 1,
    SrcColor         = 2,
    OneMinusSrcColor = 3,
    DstColor         = 4,
    OneMinusDstColor = 5,
    SrcAlpha         = 6,
    OneMinusSrcAlpha = 7,
    DstAlpha         = 8,
    OneMinusDstAlpha = 9
};

enum class BlendOp : u8 {
    Add             = 0,
    Subtract        = 1,
    ReverseSubtract = 2,
    Min             = 3,
    Max             = 4
};

enum class PolygonMode : u8 {
    Fill  = 0,
    Line  = 1,
    Point = 2
};

// NOTE: Shader Types
enum class ShaderStage : u8 {
    Vertex                 = 1 << 0,
    Fragment               = 1 << 1,
    Geometry               = 1 << 2,
    Compute                = 1 << 3,
    TessellationControl    = 1 << 4,
    TessellationEvaluation = 1 << 5
};

constexpr ShaderStage operator|(ShaderStage a, ShaderStage b) {
    return static_cast<ShaderStage>(static_cast<u8>(a) | static_cast<u8>(b));
}

constexpr bool operator&(ShaderStage a, ShaderStage b) {
    return (static_cast<u8>(a) & static_cast<u8>(b)) != 0;
}

// NOTE: Vertex Types
enum class VertexFormat : u8 {
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    UInt,
    UInt2,
    UInt3,
    UInt4,
    Byte4Norm,
    UByte4Norm
};

enum class VertexInputRate : u8 {
    PerVertex   = 0,
    PerInstance = 1
};

// NOTE: Descriptor Types
enum class DescriptorType : u8 {
    UniformBuffer      = 0,
    StorageBuffer      = 1,
    SampledTexture     = 2,
    StorageTexture     = 3,
    Sampler            = 4,
    CombinedImageSampler = 5
};

// NOTE: Render Pass Types
enum class LoadOp : u8 {
    Load   = 0,
    Clear  = 1,
    DontCare = 2
};

enum class StoreOp : u8 {
    Store  = 0,
    DontCare = 1
};

enum class PresentMode : u8 {
    Immediate    = 0,
    VSync        = 1,
    TripleBuffer = 2
};

enum class ResourceState : u8 {
    Undefined   = 0,
    ShaderRead  = 1,
    ColorWrite  = 2,
    DepthWrite  = 3,
    DepthRead   = 4,
    Present     = 5,
    TransferSrc = 6,
    TransferDst = 7
};

// NOTE: Render Graph Types
enum class PassType : u8 {
    Graphics = 0,
    Compute  = 1,
    Transfer = 2
};

enum class ResourceType : u8 {
    None       = 0,
    Texture    = 1,
    Buffer     = 2,
    Backbuffer = 3
};

// NOTE: Shader Reflection Types
enum class UniformType : u8 {
    None = 0,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat3,
    Mat4,
    Int,
    IVec2,
    IVec3,
    IVec4,
    Bool,
    Sampler2D,
    SamplerCube,
    Sampler3D
};

struct ShaderVertexAttribute {
    u32 location{0};
    const char* name{nullptr};
    UniformType type{UniformType::None};
    u32 size{0};
};

struct UniformBlockMember {
    const char* name{nullptr};
    UniformType type{UniformType::None};
    u32 offset{0};
    u32 size{0};
};

struct UniformBlock {
    const char* name{nullptr};
    u32 binding{0};
    u32 size{0};
    const UniformBlockMember* members{nullptr};
    u32 memberCount{0};
};

struct SamplerBinding {
    const char* name{nullptr};
    UniformType type{UniformType::Sampler2D};
    u32 binding{0};
};

struct ShaderReflection {
    const ShaderVertexAttribute* attributes{nullptr};
    u32 attributeCount{0};
    const UniformBlock* uniformBlocks{nullptr};
    u32 uniformBlockCount{0};
    const SamplerBinding* samplers{nullptr};
    u32 samplerCount{0};
};

// NOTE: Device Capability Structures
struct DeviceCapabilities {
    u32 maxTextureSize{0};
    u32 maxTextureUnits{0};
    u32 maxVertexAttributes{0};
    u32 maxUniformBufferSize{0};
    u32 maxStorageBufferSize{0};
    u32 maxColorAttachments{0};
    u32 maxComputeWorkGroupSize[3]{0, 0, 0};
    u32 maxComputeWorkGroupInvocations{0};
    f32 maxAnisotropy{0.0f};
    bool supportsCompute{false};
    bool supportsGeometryShader{false};
    bool supportsTessellation{false};
    bool supportsRayTracing{false};
    bool supportsBindlessTextures{false};
};

struct DeviceInfo {
    const char* vendorName{nullptr};
    const char* rendererName{nullptr};
    const char* apiVersion{nullptr};
    const char* shadingLanguageVersion{nullptr};
};

// NOTE: Configuration Structures
struct SamplerConfig {
    TextureFilter minFilter{TextureFilter::Linear};
    TextureFilter magFilter{TextureFilter::Linear};
    TextureWrap wrapU{TextureWrap::Repeat};
    TextureWrap wrapV{TextureWrap::Repeat};
    TextureWrap wrapW{TextureWrap::Repeat};
    f32 maxAnisotropy{1.0f};
    f32 borderColor[4]{0.0f, 0.0f, 0.0f, 1.0f};
};

// NOTE: Vertex Layout Types
struct VertexBinding {
    u32 binding{0};
    u32 stride{0};
    VertexInputRate inputRate{VertexInputRate::PerVertex};
};

struct VertexAttribute {
    u32 location{0};
    u32 binding{0};
    VertexFormat format{VertexFormat::Float3};
    u32 offset{0};
};

// NOTE: Descriptor Binding
struct DescriptorBinding {
    u32 binding{0};
    DescriptorType type{DescriptorType::UniformBuffer};
    ShaderStage stages{ShaderStage::Vertex};
    u32 count{1};
};

// NOTE: Pipeline State Types
struct RasterizerState {
    CullMode cullMode{CullMode::Back};
    FrontFace frontFace{FrontFace::CounterClockwise};
    PolygonMode polygonMode{PolygonMode::Fill};
    f32 lineWidth{1.0f};
    bool depthClampEnable{false};
};

struct DepthStencilState {
    bool depthTestEnable{true};
    bool depthWriteEnable{true};
    CompareOp depthCompareOp{CompareOp::Less};
    bool stencilTestEnable{false};
};

struct BlendState {
    bool enabled{false};
    BlendFactor srcColorFactor{BlendFactor::SrcAlpha};
    BlendFactor dstColorFactor{BlendFactor::OneMinusSrcAlpha};
    BlendOp colorBlendOp{BlendOp::Add};
    BlendFactor srcAlphaFactor{BlendFactor::One};
    BlendFactor dstAlphaFactor{BlendFactor::Zero};
    BlendOp alphaBlendOp{BlendOp::Add};
};

// NOTE: Viewport & Scissor
struct Viewport {
    f32 x{0.0f};
    f32 y{0.0f};
    f32 width{0.0f};
    f32 height{0.0f};
    f32 minDepth{0.0f};
    f32 maxDepth{1.0f};
};

struct Scissor {
    i32 x{0};
    i32 y{0};
    u32 width{0};
    u32 height{0};
};

// NOTE: Clear Value
struct ClearValue {
    union {
        struct { f32 r, g, b, a; } color;
        struct { f32 depth; u32 stencil; } depthStencil;
    };

    ClearValue() : color{0.0f, 0.0f, 0.0f, 1.0f} {}
    ClearValue(f32 r, f32 g, f32 b, f32 a = 1.0f) : color{r, g, b, a} {}
    ClearValue(f32 depth, u32 stencil = 0) : depthStencil{depth, stencil} {}

    static ClearValue Color(f32 r, f32 g, f32 b, f32 a = 1.0f) { return ClearValue(r, g, b, a); }
    static ClearValue Depth(f32 depth = 1.0f, u32 stencil = 0) { return ClearValue(depth, stencil); }
};

// NOTE: Render Pass Attachment Configuration
class Texture;
class Framebuffer;

struct ColorAttachmentInfo {
    Texture* texture{nullptr};
    LoadOp loadOp{LoadOp::Clear};
    StoreOp storeOp{StoreOp::Store};
    ClearValue clearValue{0.0f, 0.0f, 0.0f, 1.0f};
};

struct DepthAttachmentInfo {
    Texture* texture{nullptr};
    LoadOp loadOp{LoadOp::Clear};
    StoreOp storeOp{StoreOp::Store};
    LoadOp stencilLoadOp{LoadOp::DontCare};
    StoreOp stencilStoreOp{StoreOp::DontCare};
    ClearValue clearValue{1.0f, 0};
};

struct RenderPassBeginInfo {
    Framebuffer* framebuffer{nullptr};
    ColorAttachmentInfo* colorAttachments{nullptr};
    u32 colorAttachmentCount{0};
    DepthAttachmentInfo* depthAttachment{nullptr};
    Viewport viewport{};
    Scissor scissor{};
};

// NOTE: Command Buffer Limits
constexpr u32 MAX_DESCRIPTOR_SETS = 8;
constexpr u32 MAX_VERTEX_BINDINGS = 16;
constexpr u32 MAX_COLOR_ATTACHMENTS = 8;
constexpr u32 MAX_RENDER_PASSES = 64;

} // namespace cc::gfx
