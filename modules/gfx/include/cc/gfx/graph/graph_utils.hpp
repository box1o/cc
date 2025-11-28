#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <cc/gfx/graph/render_graph.hpp>
#include <cc/gfx/graph/render_pass.hpp>

namespace cc::gfx {

//NOTE: Builds a simple fullscreen post-process pass.
// - name: pass name
// - graph: target render graph
// - source: color texture to sample from
// - pipeline: pipeline used to draw fullscreen quad
// - descriptorSet: descriptor set bound at set index 0 (can be nullptr if bound elsewhere)
PassBuilder& AddFullscreenPass(
    RenderGraph& graph,
    std::string_view name,
    Texture* source,
    Pipeline* pipeline,
    DescriptorSet* descriptorSet = nullptr
);

} // namespace cc::gfx
