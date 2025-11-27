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

    virtual u32 GetWidth() const = 0;
    virtual u32 GetHeight() const = 0;
    virtual TextureFormat GetFormat() const = 0;
    virtual TextureType GetType() const = 0;
    virtual u32 GetHandle() const = 0;

protected:
    Texture() = default;
};

class Texture2D final : public Texture {
public:
    ~Texture2D() override;

    static scope<Texture2D> Create(Device* device, u32 width, u32 height, TextureFormat format = TextureFormat::RGBA8, const void* data = nullptr);
    static scope<Texture2D> FromFile(Device* device, const std::filesystem::path& filepath);

    void Bind(u32 slot = 0) const override;
    void Unbind() const override;

    u32 GetWidth() const override { return width_; }
    u32 GetHeight() const override { return height_; }
    TextureFormat GetFormat() const override { return format_; }
    TextureType GetType() const override { return TextureType::Texture2D; }
    u32 GetHandle() const override;

private:
    Texture2D(u32 width, u32 height, TextureFormat format, scope<TextureImpl> impl);

    u32 width_;
    u32 height_;
    TextureFormat format_;
    scope<TextureImpl> impl_;
    
    friend scope<Texture2D> CreateOpenGLTexture2D(Device*, u32, u32, TextureFormat, const void*);
    friend scope<Texture2D> CreateOpenGLTexture2DFromFile(Device*, const std::filesystem::path&);
};

class TextureCube final : public Texture {
public:
    ~TextureCube() override;

    static scope<TextureCube> Create(Device* device, u32 size, TextureFormat format = TextureFormat::RGBA8);
    static scope<TextureCube> FromFiles(Device* device, const std::array<std::filesystem::path, CUBEMAP_FACE_COUNT>& faces);

    void Bind(u32 slot = 0) const override;
    void Unbind() const override;

    u32 GetWidth() const override { return size_; }
    u32 GetHeight() const override { return size_; }
    TextureFormat GetFormat() const override { return format_; }
    TextureType GetType() const override { return TextureType::TextureCube; }
    u32 GetHandle() const override;

private:
    TextureCube(u32 size, TextureFormat format, scope<TextureImpl> impl);

    u32 size_;
    TextureFormat format_;
    scope<TextureImpl> impl_;
    
    friend scope<TextureCube> CreateOpenGLTextureCube(Device*, u32, TextureFormat);
    friend scope<TextureCube> CreateOpenGLTextureCubeFromFiles(Device*, const std::array<std::filesystem::path, CUBEMAP_FACE_COUNT>&);
};

class TextureImpl {
public:
    virtual ~TextureImpl() = default;
    virtual void Bind(u32 slot) const = 0;
    virtual void Unbind() const = 0;
    virtual u32 GetHandle() const = 0;

protected:
    TextureImpl() = default;
};

} // namespace cc::gfx
