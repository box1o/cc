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
        [[nodiscard]] scope<DescriptorSetLayout> Build();

    private:
        Device* device_{nullptr};
        std::vector<DescriptorBinding> bindings_;

        friend class DescriptorSetLayout;
    };

    virtual ~DescriptorSetLayout() = default;

    [[nodiscard]] static Builder Create(Device* device);

    [[nodiscard]] const std::vector<DescriptorBinding>& GetBindings() const noexcept { return bindings_; }
    [[nodiscard]] u32 GetBindingCount() const noexcept { return static_cast<u32>(bindings_.size()); }

    [[nodiscard]] const DescriptorBinding* GetBinding(u32 binding) const;
    [[nodiscard]] bool HasBinding(u32 binding) const;

    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

protected:
    explicit DescriptorSetLayout(std::vector<DescriptorBinding> bindings);

    std::vector<DescriptorBinding> bindings_;
};

class DescriptorSetLayoutImpl {
public:
    virtual ~DescriptorSetLayoutImpl() = default;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

protected:
    DescriptorSetLayoutImpl() = default;
};

} // namespace cc::gfx
