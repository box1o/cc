#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <vector>

namespace cc::gfx {

class VertexLayout {
public:
    class Builder {
    public:
        Builder& Binding(u32 binding, u32 stride, VertexInputRate inputRate = VertexInputRate::PerVertex);
        Builder& Attribute(u32 location, u32 binding, VertexFormat format, u32 offset);
        [[nodiscard]] scope<VertexLayout> Build();

    private:
        std::vector<VertexBinding> bindings_;
        std::vector<VertexAttribute> attributes_;
    };

    ~VertexLayout() = default;

    [[nodiscard]] static Builder Create();

    [[nodiscard]] const std::vector<VertexBinding>& GetBindings() const noexcept { return bindings_; }
    [[nodiscard]] const std::vector<VertexAttribute>& GetAttributes() const noexcept { return attributes_; }

    [[nodiscard]] u32 GetBindingCount() const noexcept { return static_cast<u32>(bindings_.size()); }
    [[nodiscard]] u32 GetAttributeCount() const noexcept { return static_cast<u32>(attributes_.size()); }

    [[nodiscard]] const VertexBinding* GetBinding(u32 binding) const;
    [[nodiscard]] u32 GetStride(u32 binding) const;

    [[nodiscard]] static u32 GetFormatSize(VertexFormat format);
    [[nodiscard]] static u32 GetFormatComponentCount(VertexFormat format);

private:
    VertexLayout(std::vector<VertexBinding> bindings, std::vector<VertexAttribute> attributes);

    std::vector<VertexBinding> bindings_;
    std::vector<VertexAttribute> attributes_;
};

} // namespace cc::gfx
