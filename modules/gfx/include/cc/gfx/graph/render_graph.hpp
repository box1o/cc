#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <cc/gfx/graph/render_pass.hpp>
#include <vector>
#include <string_view>
#include <memory>

namespace cc::gfx {

class RenderGraph {
public:
    ~RenderGraph();

    static scope<RenderGraph> Create(Device* device);

    PassBuilder& AddPass(std::string_view name, PassType type = PassType::Graphics);

    void SetSwapchain(Swapchain* swapchain);
    Swapchain* GetSwapchain() const { return swapchain_; }

    void Compile();
    void Execute();
    void Reset();

    bool IsCompiled() const { return compiled_; }
    u32 GetPassCount() const { return static_cast<u32>(passes_.size()); }

    const RenderPass* GetPass(u32 index) const;
    const RenderPass* GetPass(std::string_view name) const;

private:
    RenderGraph(Device* device);

    void BuildDependencyGraph();
    void TopologicalSort();
    void ValidateGraph();
    
    Framebuffer* GetBackbufferFramebuffer() const;
    bool HasDependency(const RenderPass* from, const RenderPass* to) const;
    bool ResourcesOverlap(const ResourceHandle& a, const ResourceHandle& b) const;

    Device* device_;
    scope<CommandBuffer> commandBuffer_;
    Swapchain* swapchain_{nullptr};

    std::vector<std::unique_ptr<RenderPass>> passes_;
    std::vector<std::unique_ptr<PassBuilder>> builders_;
    std::vector<u32> executionOrder_;

    bool compiled_{false};
};

} // namespace cc::gfx
