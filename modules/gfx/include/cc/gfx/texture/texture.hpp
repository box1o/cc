#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>
#include <filesystem>
#include <array>

namespace cc::gfx {

constexpr u32 CUBEMAP_FACE_COUNT = 6;

class Texture {
public:
    virtual ~Texture() = default;

    virtual void Bind(u32 slot = 0) const = 0;
    virtual void Unbind() const = 0;

    [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
    [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;
    [[nodiscard]] virtual TextureFormat GetFormat() const noexcept = 0;
    [[nodiscard]] virtual TextureType GetType() const noexcept = 0;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

protected:
    Texture() = default;
};

class Texture2D final : public Texture {
public:
    ~Texture2D() override;

    [[nodiscard]] static scope<Texture2D> Create(
        Device* device,
        u32 width,
        u32 height,
        TextureFormat format = TextureFormat::RGBA8,
        const void* data = nullptr
    );

    [[nodiscard]] static scope<Texture2D> FromFile(
        Device* device,
        const std::filesystem::path& filepath
    );

    void Bind(u32 slot = 0) const override;
    void Unbind() const override;

    [[nodiscard]] u32 GetWidth() const noexcept override { return width_; }
    [[nodiscard]] u32 GetHeight() const noexcept override { return height_; }
    [[nodiscard]] TextureFormat GetFormat() const noexcept override { return format_; }
    [[nodiscard]] TextureType GetType() const noexcept override { return TextureType::Texture2D; }
    [[nodiscard]] u32 GetHandle() const noexcept override;

private:
    Texture2D(u32 width, u32 height, TextureFormat format, scope<TextureImpl> impl) noexcept;

    u32 width_{0};
    u32 height_{0};
    TextureFormat format_{TextureFormat::RGBA8};
    scope<TextureImpl> impl_;

    friend scope<Texture2D> CreateOpenGLTexture2D(Device*, u32, u32, TextureFormat, const void*);
    friend scope<Texture2D> CreateOpenGLTexture2DFromFile(Device*, const std::filesystem::path&);
};

class TextureCube final : public Texture {
public:
    ~TextureCube() override;

    [[nodiscard]] static scope<TextureCube> Create(
        Device* device,
        u32 size,
        TextureFormat format = TextureFormat::RGBA8
    );

    [[nodiscard]] static scope<TextureCube> FromFiles(
        Device* device,
        const std::array<std::filesystem::path, CUBEMAP_FACE_COUNT>& faces
    );

    void Bind(u32 slot = 0) const override;
    void Unbind() const override;

    [[nodiscard]] u32 GetWidth() const noexcept override { return size_; }
    [[nodiscard]] u32 GetHeight() const noexcept override { return size_; }
    [[nodiscard]] TextureFormat GetFormat() const noexcept override { return format_; }
    [[nodiscard]] TextureType GetType() const noexcept override { return TextureType::TextureCube; }
    [[nodiscard]] u32 GetHandle() const noexcept override;

private:
    TextureCube(u32 size, TextureFormat format, scope<TextureImpl> impl) noexcept;

    u32 size_{0};
    TextureFormat format_{TextureFormat::RGBA8};
    scope<TextureImpl> impl_;

    friend scope<TextureCube> CreateOpenGLTextureCube(Device*, u32, TextureFormat);
    friend scope<TextureCube> CreateOpenGLTextureCubeFromFiles(Device*, const std::array<std::filesystem::path, CUBEMAP_FACE_COUNT>&);
};

class TextureImpl {
public:
    virtual ~TextureImpl() = default;
    virtual void Bind(u32 slot) const = 0;
    virtual void Unbind() const = 0;
    [[nodiscard]] virtual u32 GetHandle() const noexcept = 0;

protected:
    TextureImpl() = default;
};

} // namespace cc::gfx
