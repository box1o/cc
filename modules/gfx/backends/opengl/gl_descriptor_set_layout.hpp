#pragma once
#include <cc/gfx/descriptor/descriptor_set_layout.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>

namespace cc::gfx {

class Device;

class OpenGLDescriptorSetLayout final : public DescriptorSetLayout {
public:
    OpenGLDescriptorSetLayout(std::vector<DescriptorBinding> bindings);
    ~OpenGLDescriptorSetLayout() override;

    u32 GetHandle() const override { return handle_; }

private:
    u32 handle_{0};

    static inline u32 s_nextHandle{1};
};

scope<DescriptorSetLayout> CreateOpenGLDescriptorSetLayout(Device* device, const std::vector<DescriptorBinding>& bindings);

} // namespace cc::gfx
