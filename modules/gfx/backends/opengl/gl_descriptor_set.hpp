#pragma once
#include <cc/gfx/descriptor/descriptor_set.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>
#include <vector>

namespace cc::gfx {

class Device;

class OpenGLDescriptorSet final : public DescriptorSet {
public:
    OpenGLDescriptorSet(
        ref<DescriptorSetLayout> layout,
        std::vector<BufferBinding> bufferBindings,
        std::vector<TextureBinding> textureBindings
    );
    ~OpenGLDescriptorSet() override;

    void Bind(u32 setIndex) const override;
    void Update(u32 binding, ref<Buffer> buffer, u64 offset = 0, u64 range = 0) override;
    void Update(u32 binding, ref<Texture> texture, ref<Sampler> sampler = nullptr) override;

    u32 GetHandle() const override { return handle_; }

private:
    void BindBuffers(u32 setIndex) const;
    void BindTextures(u32 setIndex) const;

    u32 handle_{0};
    std::vector<BufferBinding> bufferBindings_;
    std::vector<TextureBinding> textureBindings_;

    static inline u32 s_nextHandle{1};
};

scope<DescriptorSet> CreateOpenGLDescriptorSet(
    Device* device,
    ref<DescriptorSetLayout> layout,
    const std::vector<BufferBinding>& bufferBindings,
    const std::vector<TextureBinding>& textureBindings
);

} // namespace cc::gfx
