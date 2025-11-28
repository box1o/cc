#include <cc/gfx/graph/render_graph.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/gfx/command/command_buffer.hpp>
#include <cc/gfx/framebuffer/framebuffer.hpp>
#include <cc/gfx/swapchain/swapchain.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>
#include <queue>

namespace cc::gfx {

RenderGraph::RenderGraph(Device* device)
    : device_(device) {
    commandBuffer_ = CommandBuffer::Create(device);
    log::Info("RenderGraph created");
}

RenderGraph::~RenderGraph() {
    log::Info("RenderGraph destroyed");
}

[[nodiscard]] scope<RenderGraph> RenderGraph::Create(Device* device) {
    if (device == nullptr) {
        log::Critical("Device is required to create RenderGraph");
        throw std::runtime_error("Device is null");
    }

    return scope<RenderGraph>(new RenderGraph(device));
}

PassBuilder& RenderGraph::AddPass(std::string_view name, PassType type) {
    if (compiled_) {
        log::Warn("Adding pass to compiled RenderGraph, resetting");
        Reset();
    }

    auto pass = std::make_unique<RenderPass>(std::string(name), type);
    auto builder = std::unique_ptr<PassBuilder>(new PassBuilder(pass.get()));

    PassBuilder& builderRef = *builder;

    passes_.push_back(std::move(pass));
    builders_.push_back(std::move(builder));

    log::Trace("Added pass '{}' to RenderGraph", name);

    return builderRef;
}

void RenderGraph::SetSwapchain(Swapchain* swapchain) {
    swapchain_ = swapchain;
}

Framebuffer* RenderGraph::GetBackbufferFramebuffer() const {
    if (swapchain_ == nullptr) {
        return nullptr;
    }
    return swapchain_->GetFramebuffer();
}

void RenderGraph::Compile() {
    if (passes_.empty()) {
        log::Warn("Compiling empty RenderGraph");
        compiled_ = true;
        return;
    }

    log::Info("Compiling RenderGraph with {} passes", passes_.size());

    BuildDependencyGraph();
    TopologicalSort();
    ValidateGraph();

    compiled_ = true;

    log::Info("RenderGraph compiled successfully");
    log::Trace("Execution order:");
    for (u32 i = 0; i < executionOrder_.size(); ++i) {
        log::Trace("  {}: {}", i, passes_[executionOrder_[i]]->GetName());
    }
}

void RenderGraph::Execute() {
    if (!compiled_) {
        log::Error("Cannot execute uncompiled RenderGraph");
        return;
    }

    if (passes_.empty()) {
        return;
    }

    commandBuffer_->Begin();

    for (u32 passIndex : executionOrder_) {
        const auto& pass = passes_[passIndex];

        Framebuffer* fb = pass->GetFramebuffer();

        if (fb == nullptr) {
            for (const auto& output : pass->GetOutputs()) {
                if (output.handle.IsBackbuffer()) {
                    fb = GetBackbufferFramebuffer();
                    break;
                }
            }
        }

        if (fb != nullptr) {
            commandBuffer_->BeginRenderPass(fb, pass->GetColorClear(), pass->GetDepthClear());
        }

        pass->Execute(*commandBuffer_);

        if (fb != nullptr) {
            commandBuffer_->EndRenderPass();
        }
    }

    commandBuffer_->End();
    commandBuffer_->Submit();
}

void RenderGraph::Reset() {
    passes_.clear();
    builders_.clear();
    executionOrder_.clear();
    compiled_ = false;

    log::Trace("RenderGraph reset");
}

const RenderPass* RenderGraph::GetPass(u32 index) const {
    if (index >= passes_.size()) {
        return nullptr;
    }
    return passes_[index].get();
}

const RenderPass* RenderGraph::GetPass(std::string_view name) const {
    for (const auto& pass : passes_) {
        if (pass->GetName() == name) {
            return pass.get();
        }
    }
    return nullptr;
}

void RenderGraph::BuildDependencyGraph() {
    for (auto& pass : passes_) {
        pass->executionOrder_ = 0;
    }
}

bool RenderGraph::HasDependency(const RenderPass* from, const RenderPass* to) const {
    for (const auto& input : to->GetInputs()) {
        for (const auto& output : from->GetOutputs()) {
            if (ResourcesOverlap(input.handle, output.handle)) {
                return true;
            }
        }
    }
    return false;
}

bool RenderGraph::ResourcesOverlap(const ResourceHandle& a, const ResourceHandle& b) const {
    if (a.type != b.type) {
        return false;
    }

    if (a.IsBackbuffer() && b.IsBackbuffer()) {
        return true;
    }

    return a.ptr == b.ptr;
}

void RenderGraph::TopologicalSort() {
    const u32 passCount = static_cast<u32>(passes_.size());

    std::vector<u32> inDegree(passCount, 0);
    std::vector<std::vector<u32>> adjacency(passCount);

    for (u32 i = 0; i < passCount; ++i) {
        for (u32 j = 0; j < passCount; ++j) {
            if (i != j && HasDependency(passes_[i].get(), passes_[j].get())) {
                adjacency[i].push_back(j);
                inDegree[j]++;
            }
        }
    }

    std::queue<u32> queue;
    for (u32 i = 0; i < passCount; ++i) {
        if (inDegree[i] == 0) {
            queue.push(i);
        }
    }

    executionOrder_.clear();
    executionOrder_.reserve(passCount);

    while (!queue.empty()) {
        const u32 current = queue.front();
        queue.pop();
        executionOrder_.push_back(current);

        for (u32 neighbor : adjacency[current]) {
            if (--inDegree[neighbor] == 0) {
                queue.push(neighbor);
            }
        }
    }

    if (executionOrder_.size() != passCount) {
        log::Error("Cycle detected in RenderGraph dependencies");
        throw std::runtime_error("Cycle detected in RenderGraph");
    }

    for (u32 i = 0; i < executionOrder_.size(); ++i) {
        passes_[executionOrder_[i]]->executionOrder_ = i;
    }
}

void RenderGraph::ValidateGraph() {
    for (const auto& pass : passes_) {
        if (!pass->HasExecuteCallback()) {
            log::Warn("Pass '{}' has no execute callback", pass->GetName());
        }

        const bool hasOutput = !pass->GetOutputs().empty();
        bool hasBackbufferOutput = false;

        for (const auto& output : pass->GetOutputs()) {
            if (output.handle.IsBackbuffer()) {
                hasBackbufferOutput = true;
                break;
            }
        }

        if (!hasOutput && pass->GetFramebuffer() == nullptr) {
            log::Warn("Pass '{}' has no outputs and no framebuffer", pass->GetName());
        }

        if (hasBackbufferOutput && swapchain_ == nullptr) {
            log::Warn("Pass '{}' writes to backbuffer but no swapchain is set", pass->GetName());
        }
    }
}

} // namespace cc::gfx
