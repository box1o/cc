#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <vector>

namespace cc::gfx {

class DescriptorSetLayout {
public:
    class Builder {
    public:
        Builder& Binding(u32 binding, DescriptorType type, ShaderStage stages, u32 count = 1);
        scope<DescriptorSetLayout> Build();

    private:
        Device* device_{nullptr};
        std::vector<DescriptorBinding> bindings_;

        friend class DescriptorSetLayout;
    };

    virtual ~DescriptorSetLayout() = default;

    static Builder Create(Device* device);

    const std::vector<DescriptorBinding>& GetBindings() const { return bindings_; }
    u32 GetBindingCount() const { return static_cast<u32>(bindings_.size()); }

    const DescriptorBinding* GetBinding(u32 binding) const;
    bool HasBinding(u32 binding) const;

    virtual u32 GetHandle() const = 0;

protected:
    DescriptorSetLayout(std::vector<DescriptorBinding> bindings);

    std::vector<DescriptorBinding> bindings_;
};

class DescriptorSetLayoutImpl {
public:
    virtual ~DescriptorSetLayoutImpl() = default;
    virtual u32 GetHandle() const = 0;

protected:
    DescriptorSetLayoutImpl() = default;
};

} // namespace cc::gfx
