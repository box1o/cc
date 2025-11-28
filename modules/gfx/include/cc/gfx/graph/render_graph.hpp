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

    [[nodiscard]] static scope<RenderGraph> Create(Device* device);

    PassBuilder& AddPass(std::string_view name, PassType type = PassType::Graphics);

    void SetSwapchain(Swapchain* swapchain);
    [[nodiscard]] Swapchain* GetSwapchain() const noexcept { return swapchain_; }

    void Compile();
    void Execute();
    void Reset();

    [[nodiscard]] bool IsCompiled() const noexcept { return compiled_; }
    [[nodiscard]] u32 GetPassCount() const noexcept { return static_cast<u32>(passes_.size()); }

    [[nodiscard]] const RenderPass* GetPass(u32 index) const;
    [[nodiscard]] const RenderPass* GetPass(std::string_view name) const;

private:
    explicit RenderGraph(Device* device);

    void BuildDependencyGraph();
    void TopologicalSort();
    void ValidateGraph();

    [[nodiscard]] Framebuffer* GetBackbufferFramebuffer() const;
    [[nodiscard]] bool HasDependency(const RenderPass* from, const RenderPass* to) const;
    [[nodiscard]] bool ResourcesOverlap(const ResourceHandle& a, const ResourceHandle& b) const;

    Device* device_{nullptr};
    scope<CommandBuffer> commandBuffer_;
    Swapchain* swapchain_{nullptr};

    std::vector<std::unique_ptr<RenderPass>> passes_;
    std::vector<std::unique_ptr<PassBuilder>> builders_;
    std::vector<u32> executionOrder_;

    bool compiled_{false};
};

} // namespace cc::gfx
