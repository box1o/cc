#include <cc/gfx/texture/texture.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include "backends/opengl/gl_texture.hpp"
#include <stdexcept>

namespace cc::gfx {

//NOTE: Texture2D Implementation
[[nodiscard]] scope<Texture2D> Texture2D::Create(
    Device* device,
    u32 width,
    u32 height,
    TextureFormat format,
    const void* data
) {
    if (device == nullptr) {
        log::Critical("Device is required to create Texture2D");
        throw std::runtime_error("Device is null");
    }

    switch (device->GetBackend()) {
        case Backend::OpenGL:
            return CreateOpenGLTexture2D(device, width, height, format, data);
        case Backend::Vulkan:
            log::Critical("Vulkan Texture2D backend not implemented");
            throw std::runtime_error("Vulkan Texture2D backend not implemented");
        case Backend::Metal:
            log::Critical("Metal Texture2D backend not implemented");
            throw std::runtime_error("Metal Texture2D backend not implemented");
    }

    log::Critical("Unknown backend for Texture2D::Create");
    throw std::runtime_error("Unknown backend");
}

[[nodiscard]] scope<Texture2D> Texture2D::FromFile(
    Device* device,
    const std::filesystem::path& filepath
) {
    if (device == nullptr) {
        log::Critical("Device is required to create Texture2D from file");
        throw std::runtime_error("Device is null");
    }

    switch (device->GetBackend()) {
        case Backend::OpenGL:
            return CreateOpenGLTexture2DFromFile(device, filepath);
        case Backend::Vulkan:
            log::Critical("Vulkan Texture2D::FromFile backend not implemented");
            throw std::runtime_error("Vulkan Texture2D::FromFile backend not implemented");
        case Backend::Metal:
            log::Critical("Metal Texture2D::FromFile backend not implemented");
            throw std::runtime_error("Metal Texture2D::FromFile backend not implemented");
    }

    log::Critical("Unknown backend for Texture2D::FromFile");
    throw std::runtime_error("Unknown backend");
}

Texture2D::Texture2D(u32 width, u32 height, TextureFormat format, scope<TextureImpl> impl) noexcept
    : width_(width)
    , height_(height)
    , format_(format)
    , impl_(std::move(impl)) {}

Texture2D::~Texture2D() = default;

void Texture2D::Bind(u32 slot) const {
    impl_->Bind(slot);
}

void Texture2D::Unbind() const {
    impl_->Unbind();
}

u32 Texture2D::GetHandle() const noexcept {
    return impl_->GetHandle();
}

//NOTE: TextureCube Implementation
[[nodiscard]] scope<TextureCube> TextureCube::Create(
    Device* device,
    u32 size,
    TextureFormat format
) {
    if (device == nullptr) {
        log::Critical("Device is required to create TextureCube");
        throw std::runtime_error("Device is null");
    }

    switch (device->GetBackend()) {
        case Backend::OpenGL:
            return CreateOpenGLTextureCube(device, size, format);
        case Backend::Vulkan:
            log::Critical("Vulkan TextureCube backend not implemented");
            throw std::runtime_error("Vulkan TextureCube backend not implemented");
        case Backend::Metal:
            log::Critical("Metal TextureCube backend not implemented");
            throw std::runtime_error("Metal TextureCube backend not implemented");
    }

    log::Critical("Unknown backend for TextureCube::Create");
    throw std::runtime_error("Unknown backend");
}

[[nodiscard]] scope<TextureCube> TextureCube::FromFiles(
    Device* device,
    const std::array<std::filesystem::path, CUBEMAP_FACE_COUNT>& faces
) {
    if (device == nullptr) {
        log::Critical("Device is required to create TextureCube from files");
        throw std::runtime_error("Device is null");
    }

    switch (device->GetBackend()) {
        case Backend::OpenGL:
            return CreateOpenGLTextureCubeFromFiles(device, faces);
        case Backend::Vulkan:
            log::Critical("Vulkan TextureCube::FromFiles backend not implemented");
            throw std::runtime_error("Vulkan TextureCube::FromFiles backend not implemented");
        case Backend::Metal:
            log::Critical("Metal TextureCube::FromFiles backend not implemented");
            throw std::runtime_error("Metal TextureCube::FromFiles backend not implemented");
    }

    log::Critical("Unknown backend for TextureCube::FromFiles");
    throw std::runtime_error("Unknown backend");
}

TextureCube::TextureCube(u32 size, TextureFormat format, scope<TextureImpl> impl) noexcept
    : size_(size)
    , format_(format)
    , impl_(std::move(impl)) {}

TextureCube::~TextureCube() = default;

void TextureCube::Bind(u32 slot) const {
    impl_->Bind(slot);
}

void TextureCube::Unbind() const {
    impl_->Unbind();
}

u32 TextureCube::GetHandle() const noexcept {
    return impl_->GetHandle();
}

} // namespace cc::gfx
