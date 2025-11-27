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
        scope<VertexLayout> Build();

    private:
        std::vector<VertexBinding> bindings_;
        std::vector<VertexAttribute> attributes_;
    };

    ~VertexLayout() = default;

    static Builder Create();

    const std::vector<VertexBinding>& GetBindings() const { return bindings_; }
    const std::vector<VertexAttribute>& GetAttributes() const { return attributes_; }

    u32 GetBindingCount() const { return static_cast<u32>(bindings_.size()); }
    u32 GetAttributeCount() const { return static_cast<u32>(attributes_. size()); }

    const VertexBinding* GetBinding(u32 binding) const;
    u32 GetStride(u32 binding) const;

    static u32 GetFormatSize(VertexFormat format);
    static u32 GetFormatComponentCount(VertexFormat format);

private:
    VertexLayout(std::vector<VertexBinding> bindings, std::vector<VertexAttribute> attributes);

    std::vector<VertexBinding> bindings_;
    std::vector<VertexAttribute> attributes_;
};

} // namespace cc::gfx
