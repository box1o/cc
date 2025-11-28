#include <cc/gfx/graph/graph_utils.hpp>
#include <cc/gfx/graph/resource_handle.hpp>
#include <cc/gfx/command/command_buffer.hpp>
#include <cc/gfx/pipeline/pipeline.hpp>
#include <cc/gfx/descriptor/descriptor_set.hpp>

namespace cc::gfx {

PassBuilder& AddFullscreenPass(
    RenderGraph& graph,
    std::string_view name,
    Texture* source,
    Pipeline* pipeline,
    DescriptorSet* descriptorSet
) {
    PassBuilder& pass = graph.AddPass(name, PassType::Graphics);

    if (source != nullptr) {
        pass.Read(source, ResourceState::ShaderRead);
    }

    pass.WriteBackbuffer(ResourceState::ColorWrite);
    pass.SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    pass.Execute([pipeline, descriptorSet](CommandBuffer& cmd) {
        if (pipeline == nullptr) {
            return;
        }

        cmd.BindPipeline(pipeline);

        if (descriptorSet != nullptr) {
            cmd.BindDescriptorSet(0, descriptorSet);
        }

        cmd.DrawFullscreenQuad();
    });

    return pass;
}

} // namespace cc::gfx
