#include <cc/gfx/texture/texture.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_texture.hpp"
#include <stdexcept>

namespace cc::gfx {

//NOTE: Texture2D Implementation
scope<Texture2D> Texture2D::Create(Device* device, u32 width, u32 height, TextureFormat format, const void* data) {
    if (device == nullptr) {
        log::Critical("Device is required to create Texture2D");
        throw std::runtime_error("Device is null");
    }

    return device->CreateTexture2D(width, height, format, data);
}

scope<Texture2D> Texture2D::FromFile(Device* device, const std::filesystem::path& filepath) {
    if (device == nullptr) {
        log::Critical("Device is required to create Texture2D");
        throw std::runtime_error("Device is null");
    }

    return CreateOpenGLTexture2DFromFile(device, filepath);
}

Texture2D::Texture2D(u32 width, u32 height, TextureFormat format, scope<TextureImpl> impl)
    : width_(width), height_(height), format_(format), impl_(std::move(impl)) {}

Texture2D::~Texture2D() = default;

void Texture2D::Bind(u32 slot) const {
    impl_->Bind(slot);
}

void Texture2D::Unbind() const {
    impl_->Unbind();
}

u32 Texture2D::GetHandle() const {
    return impl_->GetHandle();
}

//NOTE: TextureCube Implementation
scope<TextureCube> TextureCube::Create(Device* device, u32 size, TextureFormat format) {
    if (device == nullptr) {
        log::Critical("Device is required to create TextureCube");
        throw std::runtime_error("Device is null");
    }

    return device->CreateTextureCube(size, format);
}

scope<TextureCube> TextureCube::FromFiles(Device* device, const std::array<std::filesystem::path, CUBEMAP_FACE_COUNT>& faces) {
    if (device == nullptr) {
        log::Critical("Device is required to create TextureCube");
        throw std::runtime_error("Device is null");
    }

    return CreateOpenGLTextureCubeFromFiles(device, faces);
}

TextureCube::TextureCube(u32 size, TextureFormat format, scope<TextureImpl> impl)
    : size_(size), format_(format), impl_(std::move(impl)) {}

TextureCube::~TextureCube() = default;

void TextureCube::Bind(u32 slot) const {
    impl_->Bind(slot);
}

void TextureCube::Unbind() const {
    impl_->Unbind();
}

u32 TextureCube::GetHandle() const {
    return impl_->GetHandle();
}

} // namespace cc::gfx
