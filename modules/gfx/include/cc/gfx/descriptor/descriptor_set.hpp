#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <vector>
#include <variant>

namespace cc::gfx {

struct BufferBinding {
    u32 binding{0};
    ref<Buffer> buffer{nullptr};
    u64 offset{0};
    u64 range{0};
};

struct TextureBinding {
    u32 binding{0};
    ref<Texture> texture{nullptr};
    ref<Sampler> sampler{nullptr};
};

using DescriptorResource = std::variant<BufferBinding, TextureBinding>;

class DescriptorSet {
public:
    class Builder {
    public:
        Builder& Bind(u32 binding, ref<Buffer> buffer, u64 offset = 0, u64 range = 0);
        Builder& Bind(u32 binding, ref<Texture> texture, ref<Sampler> sampler = nullptr);
        scope<DescriptorSet> Build();

    private:
        Device* device_{nullptr};
        ref<DescriptorSetLayout> layout_{nullptr};
        std::vector<BufferBinding> bufferBindings_;
        std::vector<TextureBinding> textureBindings_;

        friend class DescriptorSet;
    };

    virtual ~DescriptorSet() = default;

    static Builder Create(Device* device, ref<DescriptorSetLayout> layout);

    ref<DescriptorSetLayout> GetLayout() const { return layout_; }

    virtual void Bind(u32 setIndex) const = 0;
    virtual void Update(u32 binding, ref<Buffer> buffer, u64 offset = 0, u64 range = 0) = 0;
    virtual void Update(u32 binding, ref<Texture> texture, ref<Sampler> sampler = nullptr) = 0;

    virtual u32 GetHandle() const = 0;

protected:
    DescriptorSet(ref<DescriptorSetLayout> layout);

    ref<DescriptorSetLayout> layout_;
};

class DescriptorSetImpl {
public:
    virtual ~DescriptorSetImpl() = default;
    virtual void Bind(u32 setIndex) const = 0;
    virtual u32 GetHandle() const = 0;

protected:
    DescriptorSetImpl() = default;
};

} // namespace cc::gfx
