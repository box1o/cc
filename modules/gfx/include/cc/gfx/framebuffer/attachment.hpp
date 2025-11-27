#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>

namespace cc::gfx {

class Texture;

enum class AttachmentType : u8 {
    Color,
    Depth,
    Stencil,
    DepthStencil
};

struct Attachment {
    AttachmentType type{AttachmentType::Color};
    TextureFormat format{TextureFormat::RGBA8};
    ref<Texture> texture{nullptr};

    static auto CreateColor(TextureFormat fmt = TextureFormat::RGBA8) -> Attachment {
        return {AttachmentType::Color, fmt, nullptr};
    }

    static auto CreateDepth(TextureFormat fmt = TextureFormat::Depth24Stencil8) -> Attachment {
        return {AttachmentType::Depth, fmt, nullptr};
    }

    static auto CreateDepthStencil() -> Attachment {
        return {AttachmentType::DepthStencil, TextureFormat::Depth24Stencil8, nullptr};
    }

    static auto WithTexture(AttachmentType type, const ref<Texture>& tex) -> Attachment;
};

} // namespace cc::gfx
