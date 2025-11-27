#pragma once
#include <cc/gfx/texture/texture.hpp>
#include <cc/gfx/types.hpp>
#include <cc/core/core.hpp>
#include <array>

namespace cc::gfx {

class Device;

class OpenGLTexture2DImpl final : public TextureImpl {
public:
    OpenGLTexture2DImpl(u32 width, u32 height, TextureFormat format, const void* data);
    ~OpenGLTexture2DImpl() override;

    void Bind(u32 slot) const override;
    void Unbind() const override;
    u32 GetHandle() const override { return handle_; }

private:
    unsigned int GetGLInternalFormat() const;
    unsigned int GetGLFormat() const;
    unsigned int GetGLType() const;

    u32 handle_{0};
    u32 width_;
    u32 height_;
    TextureFormat format_;
};

class OpenGLTextureCubeImpl final : public TextureImpl {
public:
    OpenGLTextureCubeImpl(u32 size, TextureFormat format, const std::array<const void*, CUBEMAP_FACE_COUNT>& faceData);
    ~OpenGLTextureCubeImpl() override;

    void Bind(u32 slot) const override;
    void Unbind() const override;
    u32 GetHandle() const override { return handle_; }

private:
    unsigned int GetGLInternalFormat() const;
    unsigned int GetGLFormat() const;
    unsigned int GetGLType() const;

    u32 handle_{0};
    u32 size_;
    TextureFormat format_;
};

scope<Texture2D> CreateOpenGLTexture2D(Device* device, u32 width, u32 height, TextureFormat format, const void* data);
scope<Texture2D> CreateOpenGLTexture2DFromFile(Device* device, const std::filesystem::path& filepath);

scope<TextureCube> CreateOpenGLTextureCube(Device* device, u32 size, TextureFormat format);
scope<TextureCube> CreateOpenGLTextureCubeFromFiles(Device* device, const std::array<std::filesystem::path, CUBEMAP_FACE_COUNT>& faces);

} // namespace cc::gfx
