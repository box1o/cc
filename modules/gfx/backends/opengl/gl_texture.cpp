#include "gl_texture.hpp"
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>
#include <stb_image.h>

namespace cc::gfx {

namespace {

unsigned int GetGLFilter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear:  return GL_LINEAR;
    }
    return GL_LINEAR;
}

unsigned int GetGLWrap(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat:        return GL_REPEAT;
        case TextureWrap::MirrorRepeat:  return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:   return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
    }
    return GL_REPEAT;
}

} // anonymous namespace

//NOTE: Texture2D Implementation
unsigned int OpenGLTexture2DImpl::GetGLInternalFormat() const {
    switch (format_) {
        case TextureFormat::R8:              return GL_R8;
        case TextureFormat::RG8:             return GL_RG8;
        case TextureFormat::RGB8:            return GL_RGB8;
        case TextureFormat::RGBA8:           return GL_RGBA8;
        case TextureFormat::SRGB8:           return GL_SRGB8;
        case TextureFormat::SRGB8_A8:        return GL_SRGB8_ALPHA8;
        case TextureFormat::R16F:            return GL_R16F;
        case TextureFormat::RG16F:           return GL_RG16F;
        case TextureFormat::RGB16F:          return GL_RGB16F;
        case TextureFormat::RGBA16F:         return GL_RGBA16F;
        case TextureFormat::R32F:            return GL_R32F;
        case TextureFormat::RG32F:           return GL_RG32F;
        case TextureFormat::RGB32F:          return GL_RGB32F;
        case TextureFormat::RGBA32F:         return GL_RGBA32F;
        case TextureFormat::Depth16:         return GL_DEPTH_COMPONENT16;
        case TextureFormat::Depth24:         return GL_DEPTH_COMPONENT24;
        case TextureFormat::Depth32F:        return GL_DEPTH_COMPONENT32F;
        case TextureFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        case TextureFormat::Depth32FStencil8: return GL_DEPTH32F_STENCIL8;
    }
    return GL_RGBA8;
}

unsigned int OpenGLTexture2DImpl::GetGLFormat() const {
    switch (format_) {
        case TextureFormat::R8:
        case TextureFormat::R16F:
        case TextureFormat::R32F:
            return GL_RED;

        case TextureFormat::RG8:
        case TextureFormat::RG16F:
        case TextureFormat::RG32F:
            return GL_RG;

        case TextureFormat::RGB8:
        case TextureFormat::SRGB8:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB32F:
            return GL_RGB;

        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            return GL_RGBA;

        case TextureFormat::Depth16:
        case TextureFormat::Depth24:
        case TextureFormat::Depth32F:
            return GL_DEPTH_COMPONENT;

        case TextureFormat::Depth24Stencil8:
        case TextureFormat::Depth32FStencil8:
            return GL_DEPTH_STENCIL;
    }
    return GL_RGBA;
}

unsigned int OpenGLTexture2DImpl::GetGLType() const {
    switch (format_) {
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGB16F:
        case TextureFormat::RGBA16F:
            return GL_HALF_FLOAT;

        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGB32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::Depth32F:
            return GL_FLOAT;

        case TextureFormat::Depth24Stencil8:
            return GL_UNSIGNED_INT_24_8;

        case TextureFormat::Depth32FStencil8:
            return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

        default:
            return GL_UNSIGNED_BYTE;
    }
}

OpenGLTexture2DImpl::OpenGLTexture2DImpl(u32 width, u32 height, TextureFormat format, const void* data)
: width_(width), height_(height), format_(format) {

    glCreateTextures(GL_TEXTURE_2D, 1, &handle_);
    if (handle_ == 0) {
        log::Critical("Failed to create OpenGL texture");
        return;
    }

    glTextureStorage2D(
        handle_,
        1,
        GetGLInternalFormat(),
        static_cast<int>(width_),
        static_cast<int>(height_)
    );

    if (data != nullptr) {
        glTextureSubImage2D(
            handle_,
            0,
            0, 0,
            static_cast<int>(width_),
            static_cast<int>(height_),
            GetGLFormat(),
            GetGLType(),
            data
        );
    }

    glTextureParameteri(handle_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(handle_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(handle_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(handle_, GL_TEXTURE_WRAP_T, GL_REPEAT);

    log::Trace("OpenGL Texture2D created (handle={}, {}x{})", handle_, width_, height_);
}

OpenGLTexture2DImpl::~OpenGLTexture2DImpl() {
    if (handle_ != 0) {
        glDeleteTextures(1, &handle_);
        log::Trace("OpenGL Texture2D destroyed (handle={})", handle_);
        handle_ = 0;
    }
}

void OpenGLTexture2DImpl::Bind(u32 slot) const {
    glBindTextureUnit(slot, handle_);
}

void OpenGLTexture2DImpl::Unbind() const {
    glBindTextureUnit(0, 0);
}

//NOTE: TextureCube Implementation
unsigned int OpenGLTextureCubeImpl::GetGLInternalFormat() const {
    switch (format_) {
        case TextureFormat::R8:       return GL_R8;
        case TextureFormat::RG8:      return GL_RG8;
        case TextureFormat::RGB8:     return GL_RGB8;
        case TextureFormat::RGBA8:    return GL_RGBA8;
        case TextureFormat::RGB16F:   return GL_RGB16F;
        case TextureFormat::RGBA16F:  return GL_RGBA16F;
        case TextureFormat::RGB32F:   return GL_RGB32F;
        case TextureFormat::RGBA32F:  return GL_RGBA32F;
        default: return GL_RGBA8;
    }
}

unsigned int OpenGLTextureCubeImpl::GetGLFormat() const {
    switch (format_) {
        case TextureFormat::R8:    return GL_RED;
        case TextureFormat::RG8:   return GL_RG;
        case TextureFormat::RGB8:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB32F:
            return GL_RGB;
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            return GL_RGBA;
        default: return GL_RGB;
    }
}

unsigned int OpenGLTextureCubeImpl::GetGLType() const {
    switch (format_) {
        case TextureFormat::RGB16F:
        case TextureFormat::RGBA16F:
            return GL_HALF_FLOAT;
        case TextureFormat::RGB32F:
        case TextureFormat::RGBA32F:
            return GL_FLOAT;
        default:
            return GL_UNSIGNED_BYTE;
    }
}

OpenGLTextureCubeImpl::OpenGLTextureCubeImpl(
    u32 size,
    TextureFormat format,
    const std::array<const void*, CUBEMAP_FACE_COUNT>& faceData
) : size_(size), format_(format) {

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &handle_);
    if (handle_ == 0) {
        log::Critical("Failed to create OpenGL cubemap texture");
        return;
    }

    glTextureStorage2D(
        handle_,
        1,
        GetGLInternalFormat(),
        static_cast<int>(size_),
        static_cast<int>(size_)
    );

    for (u32 i = 0; i < CUBEMAP_FACE_COUNT; ++i) {
        if (faceData[i] != nullptr) {
            glTextureSubImage3D(
                handle_,
                0,
                0, 0,
                static_cast<int>(i),
                static_cast<int>(size_),
                static_cast<int>(size_),
                1,
                GetGLFormat(),
                GetGLType(),
                faceData[i]
            );
        }
    }

    glTextureParameteri(handle_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(handle_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(handle_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(handle_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(handle_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    log::Trace("OpenGL TextureCube created (handle={}, size={})", handle_, size_);
}

OpenGLTextureCubeImpl::~OpenGLTextureCubeImpl() {
    if (handle_ != 0) {
        glDeleteTextures(1, &handle_);
        log::Trace("OpenGL TextureCube destroyed (handle={})", handle_);
        handle_ = 0;
    }
}

void OpenGLTextureCubeImpl::Bind(u32 slot) const {
    glBindTextureUnit(slot, handle_);
}

void OpenGLTextureCubeImpl::Unbind() const {
    glBindTextureUnit(0, 0);
}

//NOTE: Factory Functions
scope<Texture2D> CreateOpenGLTexture2D(
    Device* /*device*/,
    u32 width,
    u32 height,
    TextureFormat format,
    const void* data
) {
    auto impl = scope<TextureImpl>(new OpenGLTexture2DImpl(width, height, format, data));
    return scope<Texture2D>(new Texture2D(width, height, format, std::move(impl)));
}

scope<Texture2D> CreateOpenGLTexture2DFromFile(Device* device, const std::filesystem::path& filepath) {
    stbi_set_flip_vertically_on_load(1);

    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = stbi_load(filepath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (pixels == nullptr) {
        log::Error("Failed to load texture from file: {}", filepath.string());
        return CreateOpenGLTexture2D(device, 1, 1, TextureFormat::RGBA8, nullptr);
    }

    auto texture = CreateOpenGLTexture2D(
        device,
        static_cast<u32>(width),
        static_cast<u32>(height),
        TextureFormat::RGBA8,
        pixels
    );

    stbi_image_free(pixels);

    return texture;
}

scope<TextureCube> CreateOpenGLTextureCube(
    Device* /*device*/,
    u32 size,
    TextureFormat format
) {
    std::array<const void*, CUBEMAP_FACE_COUNT> faceData{};
    auto impl = scope<TextureImpl>(new OpenGLTextureCubeImpl(size, format, faceData));
    return scope<TextureCube>(new TextureCube(size, format, std::move(impl)));
}

scope<TextureCube> CreateOpenGLTextureCubeFromFiles(
    Device* device,
    const std::array<std::filesystem::path, CUBEMAP_FACE_COUNT>& faces
) {
    std::array<std::unique_ptr<unsigned char, decltype(&stbi_image_free)>, CUBEMAP_FACE_COUNT> loadedData = {
        std::unique_ptr<unsigned char, decltype(&stbi_image_free)>(nullptr, stbi_image_free),
        std::unique_ptr<unsigned char, decltype(&stbi_image_free)>(nullptr, stbi_image_free),
        std::unique_ptr<unsigned char, decltype(&stbi_image_free)>(nullptr, stbi_image_free),
        std::unique_ptr<unsigned char, decltype(&stbi_image_free)>(nullptr, stbi_image_free),
        std::unique_ptr<unsigned char, decltype(&stbi_image_free)>(nullptr, stbi_image_free),
        std::unique_ptr<unsigned char, decltype(&stbi_image_free)>(nullptr, stbi_image_free)
    };

    std::array<const void*, CUBEMAP_FACE_COUNT> faceData{};
    u32 size = 0;

    stbi_set_flip_vertically_on_load(0);

    for (u32 i = 0; i < CUBEMAP_FACE_COUNT; ++i) {
        if (!faces[i].empty()) {
            int width = 0, height = 0, channels = 0;
            unsigned char* pixels = stbi_load(
                faces[i].string().c_str(),
                &width,
                &height,
                &channels,
                STBI_rgb_alpha
            );

            if (pixels == nullptr) {
                log::Error("Failed to load cubemap face {} from file: {}", i, faces[i].string());
            } else {
                loadedData[i].reset(pixels);
                faceData[i] = pixels;
                if (i == 0) {
                    size = static_cast<u32>(width);
                }
            }
        }
    }

    //NOTE: pass faceData to actually upload pixels
    auto impl = scope<TextureImpl>(new OpenGLTextureCubeImpl(size, TextureFormat::RGBA8, faceData));
    return scope<TextureCube>(new TextureCube(size, TextureFormat::RGBA8, std::move(impl)));
}

} // namespace cc::gfx
