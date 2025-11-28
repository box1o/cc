#include <cc/gfx/pipeline/vertex_layout.hpp>
#include <cc/core/logger.hpp>
#include <stdexcept>

namespace cc::gfx {

[[nodiscard]] VertexLayout::Builder VertexLayout::Create() {
    return Builder{};
}

VertexLayout::Builder& VertexLayout::Builder::Binding(u32 binding, u32 stride, VertexInputRate inputRate) {
    VertexBinding vb{};
    vb.binding = binding;
    vb.stride = stride;
    vb.inputRate = inputRate;
    bindings_.push_back(vb);
    return *this;
}

VertexLayout::Builder& VertexLayout::Builder::Attribute(u32 location, u32 binding, VertexFormat format, u32 offset) {
    VertexAttribute attr{};
    attr.location = location;
    attr.binding = binding;
    attr.format = format;
    attr.offset = offset;
    attributes_.push_back(attr);
    return *this;
}

[[nodiscard]] scope<VertexLayout> VertexLayout::Builder::Build() {
    if (bindings_.empty()) {
        log::Error("VertexLayout requires at least one binding");
        throw std::runtime_error("VertexLayout requires at least one binding");
    }

    if (attributes_.empty()) {
        log::Error("VertexLayout requires at least one attribute");
        throw std::runtime_error("VertexLayout requires at least one attribute");
    }

    return scope<VertexLayout>(new VertexLayout(std::move(bindings_), std::move(attributes_)));
}

VertexLayout::VertexLayout(std::vector<VertexBinding> bindings, std::vector<VertexAttribute> attributes)
    : bindings_(std::move(bindings))
    , attributes_(std::move(attributes)) {
    log::Trace("VertexLayout created ({} bindings, {} attributes)", bindings_.size(), attributes_.size());
}

const VertexBinding* VertexLayout::GetBinding(u32 binding) const {
    for (const auto& b : bindings_) {
        if (b.binding == binding) {
            return &b;
        }
    }
    return nullptr;
}

u32 VertexLayout::GetStride(u32 binding) const {
    const VertexBinding* b = GetBinding(binding);
    return b != nullptr ? b->stride : 0;
}

u32 VertexLayout::GetFormatSize(VertexFormat format) {
    switch (format) {
        case VertexFormat::Float:      return 4;
        case VertexFormat::Float2:     return 8;
        case VertexFormat::Float3:     return 12;
        case VertexFormat::Float4:     return 16;
        case VertexFormat::Int:        return 4;
        case VertexFormat::Int2:       return 8;
        case VertexFormat::Int3:       return 12;
        case VertexFormat::Int4:       return 16;
        case VertexFormat::UInt:       return 4;
        case VertexFormat::UInt2:      return 8;
        case VertexFormat::UInt3:      return 12;
        case VertexFormat::UInt4:      return 16;
        case VertexFormat::Byte4Norm:  return 4;
        case VertexFormat::UByte4Norm: return 4;
    }
    return 0;
}

u32 VertexLayout::GetFormatComponentCount(VertexFormat format) {
    switch (format) {
        case VertexFormat::Float:      return 1;
        case VertexFormat::Float2:     return 2;
        case VertexFormat::Float3:     return 3;
        case VertexFormat::Float4:     return 4;
        case VertexFormat::Int:        return 1;
        case VertexFormat::Int2:       return 2;
        case VertexFormat::Int3:       return 3;
        case VertexFormat::Int4:       return 4;
        case VertexFormat::UInt:       return 1;
        case VertexFormat::UInt2:      return 2;
        case VertexFormat::UInt3:      return 3;
        case VertexFormat::UInt4:      return 4;
        case VertexFormat::Byte4Norm:  return 4;
        case VertexFormat::UByte4Norm: return 4;
    }
    return 0;
}

} // namespace cc::gfx
