#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <cc/gfx/graph/resource_handle.hpp>
#include <functional>
#include <string>
#include <vector>

namespace cc::gfx {

using PassExecuteCallback = std::function<void(CommandBuffer&)>;

class RenderPass {
public:
    RenderPass(std::string name, PassType type);
    ~RenderPass() = default;

    [[nodiscard]] const std::string& GetName() const noexcept { return name_; }
    [[nodiscard]] PassType GetType() const noexcept { return type_; }

    [[nodiscard]] Framebuffer* GetFramebuffer() const noexcept { return framebuffer_; }
    [[nodiscard]] const ClearValue& GetColorClear() const noexcept { return colorClear_; }
    [[nodiscard]] const ClearValue& GetDepthClear() const noexcept { return depthClear_; }

    [[nodiscard]] const std::vector<ResourceAccess>& GetInputs() const noexcept { return inputs_; }
    [[nodiscard]] const std::vector<ResourceAccess>& GetOutputs() const noexcept { return outputs_; }

    [[nodiscard]] u32 GetExecutionOrder() const noexcept { return executionOrder_; }
    [[nodiscard]] bool HasExecuteCallback() const noexcept { return executeCallback_ != nullptr; }

    void Execute(CommandBuffer& cmd) const;

private:
    std::string name_;
    PassType type_;
    Framebuffer* framebuffer_{nullptr};
    ClearValue colorClear_{0.0f, 0.0f, 0.0f, 1.0f};
    ClearValue depthClear_{1.0f, 0};
    std::vector<ResourceAccess> inputs_;
    std::vector<ResourceAccess> outputs_;
    PassExecuteCallback executeCallback_{};
    u32 executionOrder_{0};

    friend class PassBuilder;
    friend class RenderGraph;
};

class PassBuilder {
public:
    PassBuilder& SetFramebuffer(Framebuffer* framebuffer);
    PassBuilder& SetColorClear(const ClearValue& clearValue);
    PassBuilder& SetDepthClear(const ClearValue& clearValue);

    //NOTE: Convenience helpers
    PassBuilder& SetClearColor(f32 r, f32 g, f32 b, f32 a = 1.0f);
    PassBuilder& SetClearDepth(f32 depth = 1.0f, u32 stencil = 0);

    PassBuilder& Read(Texture* texture, ResourceState state = ResourceState::ShaderRead);
    PassBuilder& Read(Buffer* buffer, ResourceState state = ResourceState::ShaderRead);
    PassBuilder& Read(const ResourceHandle& handle, ResourceState state = ResourceState::ShaderRead);

    PassBuilder& Write(Texture* texture, ResourceState state = ResourceState::ColorWrite);
    PassBuilder& Write(Buffer* buffer, ResourceState state = ResourceState::TransferDst);
    PassBuilder& Write(const ResourceHandle& handle, ResourceState state = ResourceState::ColorWrite);

    //NOTE: Backbuffer helper
    PassBuilder& WriteBackbuffer(ResourceState state = ResourceState::ColorWrite);

    PassBuilder& Execute(PassExecuteCallback callback);

private:
    explicit PassBuilder(RenderPass* pass);

    RenderPass* pass_{nullptr};

    friend class RenderGraph;
};

} // namespace cc::gfx
