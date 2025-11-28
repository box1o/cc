#include <cc/gfx/graph/render_pass.hpp>
#include <cc/gfx/command/command_buffer.hpp>
#include <cc/core/logger.hpp>

namespace cc::gfx {

RenderPass::RenderPass(std::string name, PassType type)
    : name_(std::move(name))
    , type_(type) {
    log::Trace("RenderPass '{}' created", name_);
}

void RenderPass::Execute(CommandBuffer& cmd) const {
    if (executeCallback_) {
        executeCallback_(cmd);
    }
}

PassBuilder::PassBuilder(RenderPass* pass)
    : pass_(pass) {}

PassBuilder& PassBuilder::SetFramebuffer(Framebuffer* framebuffer) {
    pass_->framebuffer_ = framebuffer;
    return *this;
}

PassBuilder& PassBuilder::SetColorClear(const ClearValue& clearValue) {
    pass_->colorClear_ = clearValue;
    return *this;
}

PassBuilder& PassBuilder::SetDepthClear(const ClearValue& clearValue) {
    pass_->depthClear_ = clearValue;
    return *this;
}

PassBuilder& PassBuilder::Read(Texture* texture, ResourceState state) {
    return Read(ResourceHandle(texture), state);
}

PassBuilder& PassBuilder::Read(Buffer* buffer, ResourceState state) {
    return Read(ResourceHandle(buffer), state);
}

PassBuilder& PassBuilder::Read(const ResourceHandle& handle, ResourceState state) {
    if (!handle.IsValid() && !handle.IsBackbuffer()) {
        log::Warn("PassBuilder::Read called with invalid resource handle");
        return *this;
    }

    ResourceAccess access{};
    access.handle = handle;
    access.state = state;
    pass_->inputs_.push_back(access);

    return *this;
}

PassBuilder& PassBuilder::Write(Texture* texture, ResourceState state) {
    return Write(ResourceHandle(texture), state);
}

PassBuilder& PassBuilder::Write(Buffer* buffer, ResourceState state) {
    return Write(ResourceHandle(buffer), state);
}

PassBuilder& PassBuilder::Write(const ResourceHandle& handle, ResourceState state) {
    ResourceAccess access{};
    access.handle = handle;
    access.state = state;
    pass_->outputs_.push_back(access);

    return *this;
}

PassBuilder& PassBuilder::Execute(PassExecuteCallback callback) {
    pass_->executeCallback_ = std::move(callback);
    return *this;
}

} // namespace cc::gfx
