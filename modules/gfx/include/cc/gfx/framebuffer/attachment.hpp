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

    [[nodiscard]] static Attachment CreateColor(TextureFormat fmt = TextureFormat::RGBA8) {
        return {AttachmentType::Color, fmt, nullptr};
    }

    [[nodiscard]] static Attachment CreateDepth(TextureFormat fmt = TextureFormat::Depth24Stencil8) {
        return {AttachmentType::Depth, fmt, nullptr};
    }

    [[nodiscard]] static Attachment CreateDepthStencil() {
        return {AttachmentType::DepthStencil, TextureFormat::Depth24Stencil8, nullptr};
    }

    [[nodiscard]] static Attachment WithTexture(AttachmentType type, const ref<Texture>& tex);
};

} // namespace cc::gfx
