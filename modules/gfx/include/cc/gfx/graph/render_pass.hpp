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

    const std::string& GetName() const { return name_; }
    PassType GetType() const { return type_; }

    Framebuffer* GetFramebuffer() const { return framebuffer_; }
    const ClearValue& GetColorClear() const { return colorClear_; }
    const ClearValue& GetDepthClear() const { return depthClear_; }

    const std::vector<ResourceAccess>& GetInputs() const { return inputs_; }
    const std::vector<ResourceAccess>& GetOutputs() const { return outputs_; }

    u32 GetExecutionOrder() const { return executionOrder_; }
    bool HasExecuteCallback() const { return executeCallback_ != nullptr; }

    void Execute(CommandBuffer& cmd) const;

private:
    std::string name_;
    PassType type_;
    Framebuffer* framebuffer_{nullptr};
    ClearValue colorClear_{0.0f, 0.0f, 0.0f, 1.0f};
    ClearValue depthClear_{1.0f, 0};
    std::vector<ResourceAccess> inputs_;
    std::vector<ResourceAccess> outputs_;
    PassExecuteCallback executeCallback_;
    u32 executionOrder_{0};

    friend class PassBuilder;
    friend class RenderGraph;
};

class PassBuilder {
public:
    PassBuilder& SetFramebuffer(Framebuffer* framebuffer);
    PassBuilder& SetColorClear(const ClearValue& clearValue);
    PassBuilder& SetDepthClear(const ClearValue& clearValue);

    PassBuilder& Read(Texture* texture, ResourceState state = ResourceState::ShaderRead);
    PassBuilder& Read(Buffer* buffer, ResourceState state = ResourceState::ShaderRead);
    PassBuilder& Read(const ResourceHandle& handle, ResourceState state = ResourceState::ShaderRead);

    PassBuilder& Write(Texture* texture, ResourceState state = ResourceState::ColorWrite);
    PassBuilder& Write(Buffer* buffer, ResourceState state = ResourceState::TransferDst);
    PassBuilder& Write(const ResourceHandle& handle, ResourceState state = ResourceState::ColorWrite);

    PassBuilder& Execute(PassExecuteCallback callback);

private:
    PassBuilder(RenderPass* pass);

    RenderPass* pass_;

    friend class RenderGraph;
};

} // namespace cc::gfx
